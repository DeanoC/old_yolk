/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\vmthread.cpp

 Implements the vmthread class.
 */
#include "dwm.h"
#include "isolatedexecengine.h"
#include "vmthread.h"

VMThread::VMThread( Dwm& dwm ) :
	  world( dwm ) {

	execEngine.reset( CORE_NEW IsolatedExecEngine( 4 * IsolatedExecEngine::MiB, 1 * IsolatedExecEngine::MiB) );	  	
}
