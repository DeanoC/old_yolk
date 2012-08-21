/*
*/
#include "core/core.h"
#include "core/fileio.h"
#include "core/file_path.h"

#include "llvm/LLVMContext.h"
#include "llvm/PassManager.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetAsmParser.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Linker.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/IntrinsicLowering.h" // @LOCALMOD
#include "llvm/LinkAllPasses.h"
#include "llvm/Support/SourceMgr.h"

#include "bitcoder.h"

namespace {
	class InternalizePass : public llvm::ModulePass {
	public:
		static char ID; // Pass identification, replacement for typeid
		explicit InternalizePass(const std::vector<std::string>& exportList);
		virtual bool runOnModule(llvm::Module &M);

		virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
		  AU.setPreservesCFG();
		  AU.addPreserved<llvm::CallGraph>();
		}
	private:
		std::set<std::string> ExternalNames;
	};

	char InternalizePass::ID = 0;
	InternalizePass::InternalizePass(const std::vector<std::string>& exportList)
	  : ModulePass(ID) {
			using namespace llvm;
			initializeInternalizePassPass(*PassRegistry::getPassRegistry());
			for(std::vector<std::string>::const_iterator itr = exportList.cbegin(); itr != exportList.cend(); itr++) {
				ExternalNames.insert(*itr);
			}

			const llvm::StringSet<> &IntrinsicSymbols = IntrinsicLowering::GetFuncNames();
			for (llvm::StringSet<>::const_iterator it = IntrinsicSymbols.begin(), ie = IntrinsicSymbols.end(); it != ie; ++it) {
				ExternalNames.insert( it->getKey().str() );
			}

			ExternalNames.insert("__stack_chk_fail");
			ExternalNames.insert("llvm.used");
			ExternalNames.insert("llvm.compiler.used");
			ExternalNames.insert("llvm.global_ctors");
			ExternalNames.insert("llvm.global_dtors");
			ExternalNames.insert("llvm.global.annotations");
			ExternalNames.insert("__stack_chk_guard");
			ExternalNames.insert("__libc_init");
			ExternalNames.insert("__libc_init_common");
			ExternalNames.insert("_init_thread");

	}

	bool InternalizePass::runOnModule(llvm::Module &M) {
		using namespace llvm;
		CallGraph *CG = getAnalysisIfAvailable<CallGraph>();
		CallGraphNode *ExternalNode = CG ? CG->getExternalCallingNode() : 0;
	  
		if (ExternalNames.empty()) {
			return false;
		}

		bool Changed = false;

		// Mark all functions not in the api as internal.
		// FIXME: maybe use private linkage?
		for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
			if (!I->isDeclaration() &&         // Function must be defined here
				!I->hasAvailableExternallyLinkage() &&
				!I->hasLocalLinkage() &&  // Can't already have internal linkage
				!ExternalNames.count(I->getName())) {// Not marked to keep external?

				I->setLinkage(GlobalValue::InternalLinkage);
				// Remove a callgraph edge from the external node to this function.
				if (ExternalNode) {
					ExternalNode->removeOneAbstractEdgeTo((*CG)[I]);
				}
				Changed = true;
//				LOG(INFO) << "Internalizing func " << I->getName().str() << "\n";
			}
		}

		// Mark all global variables with initializers that are not in the api as
		// internal as well.
		// FIXME: maybe use private linkage?
		for (Module::global_iterator I = M.global_begin(), E = M.global_end(); I != E; ++I) {
			if (!I->isDeclaration() && !I->hasLocalLinkage() &&
				!I->hasAvailableExternallyLinkage() &&
				!ExternalNames.count(I->getName())) {

				I->setLinkage(GlobalValue::InternalLinkage);
				Changed = true;
//				LOG(INFO) << "Internalized gvar " << I->getName().str() << "\n";
			}
		}

		// Mark all aliases that are not in the api as internal as well.
		for (Module::alias_iterator I = M.alias_begin(), E = M.alias_end();I != E; ++I) {
			if (!I->isDeclaration() && !I->hasInternalLinkage() &&
			    // Available externally is really just a "declaration with a body".
			    !I->hasAvailableExternallyLinkage() &&
			    !ExternalNames.count(I->getName())) {

				I->setLinkage(GlobalValue::InternalLinkage);
				Changed = true;
//				LOG(INFO) << "Internalized alias " << I->getName().str() << "\n";
			}
		}
		
		return Changed;
	}
} // end anonymous namespace

BitCoder::BitCoder() :
	trustedTriple( "x86_64-unknown-linux" ),
//	untrustedTriple( "x86_64-unknown-nacl" )
	untrustedTriple( "x86_64-unknown-linux" ) // debug
{
	using namespace llvm;

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	const std::string cpu( "bdver1" );
	// Package up features to be passed to target/subtarget
	std::string featuresStr;
	SubtargetFeatures Features;
	TargetOptions toptions;

	// debug options
	toptions.NoFramePointerElim = true;
	toptions.NoFramePointerElimNonLeaf = true;
	toptions.JITEmitDebugInfo = true;
	toptions.JITExceptionHandling = true;
	CodeGenOpt::Level optLevel = CodeGenOpt::Less;

	// TODO de memory leak
	std::string Error;
	const Target* tUNTRUSTED = TargetRegistry::lookupTarget( untrustedTriple.getTriple(), Error );
	if( !Error.empty() ) { LOG(ERROR) << Error << "\n"; }
	const Target* tTRUSTED = TargetRegistry::lookupTarget( trustedTriple.getTriple(), Error );
	if( !Error.empty() ) { LOG(ERROR) << Error << "\n"; }

	Features.getDefaultSubtargetFeatures( untrustedTriple );
	featuresStr = Features.getString();
  	tm[UNTRUSTED].reset( tUNTRUSTED->createTargetMachine( untrustedTriple.getTriple(), cpu, featuresStr, toptions, Reloc::Static, CodeModel::Large, optLevel ) );
	mcii[UNTRUSTED].reset( tUNTRUSTED->createMCInstrInfo() );
	mcsti[UNTRUSTED].reset( tUNTRUSTED->createMCSubtargetInfo( untrustedTriple.getTriple(), cpu, featuresStr ) );
  	mcai[UNTRUSTED].reset( tUNTRUSTED->createMCAsmInfo( untrustedTriple.getTriple() ) );
  	mcri[UNTRUSTED].reset( tUNTRUSTED->createMCRegInfo( untrustedTriple.getTriple() ) );
	mcab[UNTRUSTED] = tUNTRUSTED->createMCAsmBackend( untrustedTriple.getTriple() ) ;
	Features.getDefaultSubtargetFeatures( trustedTriple );
	featuresStr = Features.getString();
  	tm[TRUSTED].reset( tTRUSTED->createTargetMachine( trustedTriple.getTriple(), cpu, featuresStr, toptions, Reloc::Static, CodeModel::Large, optLevel ) );
	mcii[TRUSTED].reset( tTRUSTED->createMCInstrInfo() );
	mcsti[TRUSTED].reset( tTRUSTED->createMCSubtargetInfo( trustedTriple.getTriple(), cpu, featuresStr ) );
  	mcai[TRUSTED].reset( tTRUSTED->createMCAsmInfo( trustedTriple.getTriple() ) );
  	mcri[TRUSTED].reset( tTRUSTED->createMCRegInfo( trustedTriple.getTriple() ) );
	mcab[TRUSTED] = tTRUSTED->createMCAsmBackend( trustedTriple.getTriple() );

}
/*
void BitCoder::addLibrary( llvm::Module* lib ) {	
	libraries.push_back( lib );
}
void BitCoder::removeLibrary( llvm::Module* lib ) {	
	libraries.remove( lib );
	CORE_DELETE lib;
}
*/

std::string BitCoder::make( const int type, llvm::Module* prg ) {
	using namespace llvm;
	std::string output;
	raw_string_ostream os(output);
    formatted_raw_ostream fos(os);

    prg->setOutputFormat( Module::ExecutableOutputFormat );

	// link together everything
/*	Linker linker( StringRef("bitcoder"), prg, Linker::Verbose);
	auto lkIt = libraries.begin();
	while( lkIt != libraries.end() ) {
		linker.LinkInModule( *lkIt, Linker::PreserveSource );
		++lkIt;
	}
	Module* bc = linker.getModule(); // link result in bc owned by Linker
	*/
	Module* bc = prg;
    bc->setOutputFormat(  Module::ExecutableOutputFormat );

	std::vector<std::string> mustKeep;
	mustKeep.push_back( "main" );

	// LTO passes 
	PassManager pm;
	pm.add( CORE_NEW TargetData( *tm[type]->getTargetData() ) );
	pm.add( CORE_NEW InternalizePass( mustKeep ) );
	PassManagerBuilder().populateLTOPassManager(pm, false, false, false);
	FunctionPassManager codeGenPasses(bc);
	codeGenPasses.add( CORE_NEW TargetData( *tm[type]->getTargetData() ) );
	if( tm[type]->addPassesToEmitFile( pm, fos, TargetMachine::CGFT_ObjectFile, false ) ) {
		LOG(INFO) << "ERR\n";
	}

	pm.run( *bc );
	// code generator pass
  	codeGenPasses.doInitialization();
	for (Module::iterator it = bc->begin(), e = bc->end(); it != e; ++it) {
		if (!it->isDeclaration()) {
			codeGenPasses.run(*it);
		}
	}
	codeGenPasses.doFinalization();

	fos.flush();
	os.flush();

//	LOG(INFO) << output << "\n";
	return output;
}

llvm::Module* BitCoder::loadBitCode( const Core::FilePath& filepath ) {
	using namespace Core;

	File bcFile;
	
	if( bcFile.open( filepath.value().c_str() ) == false ) {
		// file not found
		return nullptr;
	}
   return loadBitCode( bcFile );
}

llvm::Module* BitCoder::loadBitCode( Core::InOutInterface& inny ) {
	using namespace Core;
	using namespace llvm;

	uint64_t bcLen = inny.bytesLeft();
	std::string errMsg;

	// note on 32 bit system only load max 32 bit file size (no harm)
	MemoryBuffer *bcBuffer = MemoryBuffer::getNewMemBuffer( (size_t) bcLen );
	inny.read( (uint8_t*) bcBuffer->getBuffer().data(), (size_t) bcLen );
	llvm::Module* mod = llvm::ParseBitcodeFile( bcBuffer, llvm::getGlobalContext(), &errMsg );

	return mod;
}
std::string BitCoder::assemble( const int type, const Core::FilePath& filepath ) {
	using namespace Core;

	File asmFile;
	
	if( asmFile.open( filepath.value().c_str() ) == false ) {
		// file not found
		return std::string();
	}
   return assemble( type, asmFile );
}
std::string BitCoder::assemble( const int type, Core::InOutInterface& inny ) {
	using namespace Core;
	using namespace llvm;
	using std::unique_ptr;

	uint64_t asmLen = inny.bytesLeft();

	// note on 32 bit system only load max 32 bit file size (no harm)
	MemoryBuffer *asmBuffer = MemoryBuffer::getNewMemBuffer( (size_t) asmLen );
	inny.read( (uint8_t*) asmBuffer->getBuffer().data(), (size_t) asmLen );

	SourceMgr srcMgr;
	srcMgr.AddNewSourceBuffer( asmBuffer, SMLoc());

	std::string output;
	raw_string_ostream os(output);
    formatted_raw_ostream fos(os);

	unique_ptr<MCObjectFileInfo> mcofi( CORE_NEW MCObjectFileInfo() );
	MCContext ctx(*mcai[type], *mcri[type], mcofi.get(), &srcMgr);
	mcofi->InitMCObjectFileInfo( tm[type]->getTargetTriple(), Reloc::Static, CodeModel::Small, ctx );
	MCCodeEmitter* mcce( tm[type]->getTarget().createMCCodeEmitter( *mcii[type], *mcri[type], *mcsti[type], ctx ) );

	unique_ptr<MCStreamer> streamer( tm[type]->getTarget().createMCObjectStreamer(	
										tm[type]->getTargetTriple(), ctx, 
										*mcab[type], fos, mcce, true, true ) );
	unique_ptr<MCAsmParser> parser( createMCAsmParser(	srcMgr, 
														ctx, 
														*streamer, 
														*tm[type]->getMCAsmInfo() ) );
	unique_ptr<MCTargetAsmParser> tap( tm[type]->getTarget().createMCAsmParser( *mcsti[type], *parser ) );

	parser->setTargetParser( *tap.get() );

	parser->Run(false);

	fos.flush();
	os.flush();

	return output;
}

