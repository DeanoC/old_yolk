/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\vmThreads.h

 Declares the vm thread class.
 */
#pragma once
#ifndef DWM_VMTHREADS_H_
#define DWM_VMTHREADS_H_
 
#include "isolatedexecengine.h"
#include "dwm.h"

class VMThreads {
public:
	VMThreads( Dwm& dwm );

	void run( const std::string& prg );

	WorldPtr getWorld() const { return dwm.getWorld(); }
private:
	Dwm&										dwm;
	std::unique_ptr<IsolatedExecEngine>			execEngine;
};

#endif