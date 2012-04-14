
#include "core/core.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include <cerrno>
#include "json_spirit/json_spirit_reader.h"
using namespace llvm;

static ExecutionEngine *EE = 0;

// Yolkside
//
	 
/**
	Defines an alias representing the yolk security.
	| 1 bit | 7 bit        | 24 bit          |
	| Super | local sec id | local object id |
	*/
typedef uint32_t YolkSec;

/**
	Defines an alias representing the yolk type of object.
	*/
typedef uint32_t YolkObjectType;

class YolkModule {
public:
	llvm::Module* ir;

	std::vector< const char* >		exportNames;
	std::vector< llvm::Function* >	exportsFuncs;
};

class YolkApi {
public:
	std::list< YolkModule* >	modules;
	std::list< uint32_t >		functionOrdinals;
};

/**
	Defines an alias representing the yolk api table.
	*/
typedef std::vector<YolkApi*> YolkApiTable;
	
// Per Area
// upto 24 bit (4Mib) local object
struct YolkAreaHandle {
	YolkSec			sec;
	YolkObjectType	type;
	YolkApiTable*	apiTable;	
};

// VT side
typedef uint32_t VT_SecHandle;
typedef uint32_t VT_ApiName;
typedef std::vector<void*> VT_ApiTable;

/**
	Strand local storage. 
	A strand is like a thread, but local to a object. So its an object specific code
	path local storage.
	*/	 
struct StrandLocalStorage {
};
/**
	Strand pro store. StrandLocalStorage thats read only and contains vital gubbins
	that ensure security and access cross the yolk wall.
	Read Only to VT
	*/
struct StrandProStore {
	VT_SecHandle		sec;
	VT_ApiTable			apiTable;
};
/*
// cross the yolk wall and insert a security valided api func table into the objects StrandLocal
// only need once per strand but benign if called multiple times
DVM_ApiAccess( VT_ApiName name );

VT_ModuleHandle DVM_LoadIR( const char* irFileName );

VT_ApiHandle DVM_ApiCreate( VT_ApiName name, unsigned int secLevl );
void DVM_ApiAddFunction( VT_ApiHandle api, VT_ModuleHandle module, const char* funcName );
*/
/*
static RO StrandProStore* sls;
void CallApi( API, FUNC )
{
	if( sls->apiTable[ API ] == nullptr ) {
		DWM_ApiAccess( API );
	}

	assert( sls->apiTable[ API ] != nullptr );
	assert( sls->apiTable[ API ][ FUNC ] != nullptr );

	sls->apiTable[ API ][ FUNC ]( );
}
*/

/* VT API def JSON
Unique API ID IFFtag
Sec level requirements
Description
list of IR files
for each IR File 
	list of exports
---
{
	"ApiId":	"COR0",
	"SecLvl":	"0",
	"Desc":		"Core functions available to everyone",
	"IrFiles":	[ 
		{ "File": "cor0_printf.bc", "Exports": [ "printf", "sprintf", "fprintf" ] }	
	],

}
*/
/*

YolkREMFragment* LoadIR( const char* irFileName ) {
	LLVMContext &Context = getGlobalContext();

	// Load the bitcode...
	SMDiagnostic Err;
	Module *Mod = ParseIRFile( irFileName, Err, Context);
	if (!Mod) {
		Err.print( "lli_test", errs());
		return nullptr;
	}

	std::string ErrorMsg;
	if (Mod->MaterializeAllPermanently(&ErrorMsg)) {
		errs() << ": bitcode didn't read correctly.\n";
		errs() << "Reason: " << ErrorMsg << "\n";
		exit(1);
	}

	YolkREMFragment* newFrag = new YolkREMFragment();
	newFrag->ir = Mod;

	return newFrag;
}

//===----------------------------------------------------------------------===//
// main Driver function
//
int CallREM( const char* irFileName ) {

	LLVMContext &Context = getGlobalContext();

	auto frag = loadIR( irFileName );
	Module *Mod = frag->ir;

	std::string ErrorMsg;
	EngineBuilder builder(Mod);
	builder.setErrorStr(&ErrorMsg);
	builder.setEngineKind( EngineKind::Interpreter );
	EE = builder.create();
	if (!EE) {
		if (!ErrorMsg.empty())
			errs() << ": error creating EE: " << ErrorMsg << "\n";
		else
			errs() << ": unknown error creating EE!\n";
		exit(1);
	}

	Function *EntryFn = Mod->getFunction("REM_test");
	if (!EntryFn) {
		errs() << '\'' << "\' function not found in module.\n";
		return -1;
	}

	// Run static constructors.
	EE->runStaticConstructorsDestructors(false);

	for (Module::iterator I = Mod->begin(), E = Mod->end(); I != E; ++I) {
		Function *Fn = &*I;
		if (Fn != EntryFn && !Fn->isDeclaration())
			EE->getPointerToFunction(Fn);
	}

	// Run main.
	std::vector<GenericValue> GVArgs;
	GVArgs.push_back( PTOGV(nullptr) ); // context*
	GVArgs.push_back( PTOGV(nullptr) ); // REM instance store*
	EE->runFunction(EntryFn, GVArgs);

	// Run static destructors.
	EE->runStaticConstructorsDestructors(true);

	delete EE;
	llvm_shutdown();

	return 0;
}
*/