/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\vmThread.h

 Declares the vm thread class.
 */
#pragma once
#ifndef DWM_VMTHREAD_H_
#define DWM_VMTHREAD_H_
 
#include "isolatedexecengine.h"

class VMThread : public Core::thread {
public:
	VMThread( Dwm& dwm, llvm::Module* module );

private:
	Dwm&										world;
	Core::scoped_ptr<IsolatedExecEngine>		execEngine;
};

#endif