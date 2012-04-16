#include "dwm.h"
#include "llvm/Support/raw_ostream.h"

#include "isolatedexecengine.h"

IsolatedExecEngine::IsolatedExecEngine( llvm::Module* init ) {
	using namespace llvm;

   std::string dbgstring;
   init->print( llvm::raw_string_ostream(dbgstring), 0 );
   Log << dbgstring.c_str();

	EngineBuilder builder( init );
	// TODO enable JIT
	builder.setEngineKind( EngineKind::Interpreter ); 
	exec.reset( builder.create() );

}

void IsolatedExecEngine::run( const char* funcname ) {
   using namespace llvm;

   Function *EntryFn = exec->FindFunctionNamed( funcname );

   std::vector<GenericValue> GVArgs;

//   GVArgs.push_back( world.getContext() ); // context*
//   GVArgs.push_back( PTOGV(nullptr) ); // REM instance store*
   exec->runFunction(EntryFn, GVArgs);
}