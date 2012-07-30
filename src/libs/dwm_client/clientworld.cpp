//!-----------------------------------------------------
//!
//! \file clientworld.cpp
//!
//!-----------------------------------------------------
#include <core/core.h>
#include "dwm/trustedregion.h"
#include "clientworld.h"

extern void InstallInputApiFuncs( TrustedRegion* trustedRegion );
extern void InstallCamApiFuncs( TrustedRegion* trustedRegion );

ClientWorld::ClientWorld() :
	inputQueue( 3 ) {
}

void ClientWorld::InstallApiFuncs( TrustedRegion* trustedRegion ) {
	World::InstallApiFuncs( trustedRegion );
	InstallInputApiFuncs( trustedRegion );
	InstallCamApiFuncs( trustedRegion );
}

void ClientWorld::queueInputFrame( const InputFrame& frame ) {
	inputQueue.push_front( frame );
}

bool ClientWorld::dequeueInputFrame( InputFrame* frame ) {
	if( inputQueue.isNotEmpty() ) {
		inputQueue.pop_back( frame );
		return true;
	} else {
		return false;
	}
}


