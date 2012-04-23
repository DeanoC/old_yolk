/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\vmthread.cpp

 Implements the vmthread class.
 */
#include "dwm.h"
#include "isolatedexecengine.h"
#include "vmthread.h"

VMThread::VMThread( Dwm& dwm, llvm::Module* module ) :
	  world( dwm ),
	  execEngine( CORE_NEW IsolatedExecEngine( module ) ) {
}