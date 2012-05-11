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
	VMThread( Dwm& dwm );
	IsolatedExecEngine* getEngine() { return execEngine.get(); }

private:
	Dwm&										world;
	boost::scoped_ptr<IsolatedExecEngine>		execEngine;
};

#endif