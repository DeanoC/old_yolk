#include "dwm.h"
#include "llvm/Support/raw_ostream.h"

#include "isolatedexecengine.h"

IsolatedExecEngine::IsolatedExecEngine( llvm::Module* init ) :
   llvm::Interpreter( init )
{
	using namespace llvm;

   std::string dbgstring;
   init->print( llvm::raw_string_ostream(dbgstring), 0 );
   Log << dbgstring.c_str();

}

void IsolatedExecEngine::run( const char* funcname, const std::vector<llvm::GenericValue>& args ) {
   llvm::Function* fn = FindFunctionNamed( funcname );
   runFunction( fn, args );
}

void DebugOutFn( const char* txt ) {
   Log << txt;
}

llvm::GenericValue IsolatedExecEngine::callExternalFunction(llvm::Function *F,
                                     const std::vector<llvm::GenericValue> &ArgVals) {
   using namespace llvm;
   if( std::string( F->getName().data() ) == "DebugOut" ) {
      DebugOutFn( (const char*) ArgVals[0].PointerVal );
   } else {
      Log << "External Function called : " << F->getName().data();
   }

   return GenericValue();
}