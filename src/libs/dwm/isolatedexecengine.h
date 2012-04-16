/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\isolatedexecengine.h

 Declares the isolatedexecengine class.
 */
 
#ifndef DWM_ISOLATEDEXECENGINE_H_
#define DWM_ISOLATEDEXECENGINE_H_

class IsolatedExecEngine {
public:
	IsolatedExecEngine( llvm::Module* init );

private:
	Core::scoped_ptr<llvm::ExecutionEngine> exec;
};

#endif