/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\vmthreads.cpp

 Implements the vmthread class.
 */
#include "dwm.h"
#include "isolatedexecengine.h"
#include "vmthreads.h"
#include "world.h"

extern void InstallDWMApiFuncs( TrustedRegion* trustedRegion );

VMThreads::VMThreads( Dwm& _dwm ) :
	dwm( _dwm ) {

	execEngine.reset( CORE_NEW IsolatedExecEngine(	64 * IsolatedExecEngine::MiB, // memory size include stack + trusted
													1 * IsolatedExecEngine::MiB, // stack size 
													64 * IsolatedExecEngine::KiB, // trusted region size
													getWorld().get() ) );	  	
	execEngine->addLibrary( dwm.switcherElf );
}

void VMThreads::run( const std::string& prg ) {

   execEngine->process( prg, &InstallDWMApiFuncs );

}