/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\isolatedexecengine.h

 Declares the isolatedexecengine class.
 */
 
#ifndef DWM_ISOLATEDEXECENGINE_H_
#define DWM_ISOLATEDEXECENGINE_H_

#include "../lib/ExecutionEngine/Interpreter/Interpreter.h"

class IsolatedExecEngine : public llvm::Interpreter {
public:
	explicit IsolatedExecEngine( llvm::Module* init );

   void run( const char* funcname, const std::vector<llvm::GenericValue>& args );

   virtual llvm::GenericValue callExternalFunction(llvm::Function *F,
                                 const std::vector<llvm::GenericValue> &ArgVals);

};

#endif