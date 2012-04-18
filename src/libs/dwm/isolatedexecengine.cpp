#include "dwm.h"
#include "llvm/Support/raw_ostream.h"

#include "isolatedexecengine.h"

IsolatedExecEngine::IsolatedExecEngine( llvm::Module* init ) :
   llvm::Interpreter( init )
{
	using namespace llvm;

//   std::string dbgstring;
//   init->print( llvm::raw_string_ostream(dbgstring), 0 );
//   Log << dbgstring.c_str();

   auto fn = init->getFunctionList().begin();
   while( fn != init->getFunctionList().end() ) {
      if( fn->isDeclaration() ) {
         Log << "extern " << fn->getName().data() << Core::Logger::endl;
         //TODO prep external function map here
      }
      ++fn;
   }

}

void IsolatedExecEngine::run( const char* funcname, const std::vector<llvm::GenericValue>& args ) {
   llvm::Function* fn = FindFunctionNamed( funcname );
   runFunction( fn, args );
}

llvm::GenericValue DebugOutFn( const std::vector<llvm::GenericValue> &ArgVals ) {
   Log << (const char*) ArgVals[0].PointerVal;
   return llvm::GenericValue();
}

llvm::GenericValue IsolatedExecEngine::callExternalFunction(llvm::Function *F,
                                     const std::vector<llvm::GenericValue> &ArgVals) {
   using namespace llvm;
   auto fn = extFnHash.find( (uintptr_t) F );
   if( fn == extFnHash.end() ) {
      if( std::string( F->getName().data() ) == "printf" ) {
         extFnHash[ (uintptr_t) F ] = DebugOutFn;
         fn = extFnHash.find( (uintptr_t) F );
      } else {
         Log << "External Function called : " << F->getName().data() << " < " << (uintptr_t)F << " >" << Core::Logger::endl;
         return GenericValue();
      }
   }
   return fn->second( ArgVals );
}