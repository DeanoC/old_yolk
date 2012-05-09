/*
*/
#include "core/core.h"
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
#include <fstream>
#include <set>
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/IntrinsicLowering.h" // @LOCALMOD
#include "llvm/LinkAllPasses.h"
#include "bitcoder.h"

namespace {
	class InternalizePass : public llvm::ModulePass {
	public:
		static char ID; // Pass identification, replacement for typeid
		explicit InternalizePass(const std::vector<const std::string>& exportList);
		virtual bool runOnModule(llvm::Module &M);

		virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
		  AU.setPreservesCFG();
		  AU.addPreserved<llvm::CallGraph>();
		}
	private:
		std::set<std::string> ExternalNames;
	};

	char InternalizePass::ID = 0;
	InternalizePass::InternalizePass(const std::vector<const std::string>& exportList)
	  : ModulePass(ID) {
			using namespace llvm;
			initializeInternalizePassPass(*PassRegistry::getPassRegistry());
			for(std::vector<const std::string>::const_iterator itr = exportList.begin(); itr != exportList.end(); itr++) {
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
				LOG(INFO) << "Internalizing func " << I->getName().str() << "\n";
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
				LOG(INFO) << "Internalized gvar " << I->getName().str() << "\n";
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
				LOG(INFO) << "Internalized alias " << I->getName().str() << "\n";
			}
		}
		return Changed;
	}
} // end anonymous namespace

BitCoder::BitCoder() :
	triple( "i686-unknown-nacl" )
{
	using namespace llvm;

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmPrinters();

	// TODO de memory leak
	std::string Error;
	const Target* target = TargetRegistry::lookupTarget( triple.getTriple(), Error);
	if( !Error.empty() ) {
	   LOG(ERROR) << Error << "\n";
	}

	const std::string cpu( "core2" );
	// Package up features to be passed to target/subtarget
	std::string featuresStr;
	SubtargetFeatures Features;
	Features.getDefaultSubtargetFeatures( triple );
	featuresStr = Features.getString();
	
  	tm.reset( target->createTargetMachine( triple.getTriple(), cpu, featuresStr, TargetOptions() ) );

}

void BitCoder::addLibrary( std::shared_ptr<llvm::Module> lib ) {	
	libraries.push_back( lib );
}
void BitCoder::removeLibrary( std::shared_ptr<llvm::Module> lib ) {	
	libraries.remove( lib );
}

std::string BitCoder::make( std::shared_ptr<llvm::Module> prg ) {
	using namespace llvm;
	std::string output;
	raw_string_ostream os(output);
    formatted_raw_ostream fos(os);

    prg->setOutputFormat( Module::ExecutableOutputFormat );

	// link together everything
	Linker linker( StringRef("bitcoder"), prg.get(), Linker::Verbose);
	auto lkIt = libraries.begin();
	while( lkIt != libraries.end() ) {
		linker.LinkInModule( lkIt->get(), Linker::PreserveSource );
		++lkIt;
	}
	Module* bc = linker.getModule(); // link result in bc owned by Linker
    bc->setOutputFormat(  Module::ExecutableOutputFormat );

	std::vector<const std::string> mustKeep;
	mustKeep.push_back( "main" );

	// LTO passes 
	PassManager pm;
	pm.add( CORE_NEW TargetData( *tm->getTargetData() ) );
	pm.add( CORE_NEW InternalizePass( mustKeep ) );
	PassManagerBuilder().populateLTOPassManager(pm, false, false, false);
	FunctionPassManager codeGenPasses(bc);
	codeGenPasses.add( CORE_NEW TargetData( *tm->getTargetData() ) );
	if( tm->addPassesToEmitFile( pm, fos, TargetMachine::CGFT_ObjectFile, false ) ) {
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
