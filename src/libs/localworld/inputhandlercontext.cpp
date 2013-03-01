//!-----------------------------------------------------
//!
//! \file inputhandlercontext.cpp
//!
//!-----------------------------------------------------
#include "localworld.h"
#include <scene/camera.h>
#include <scene/rendercontext.h>
#include "sceneworld.h"
#include "inputhandlercontext.h"

InputHandlerContext::InputHandlerContext() :
	inputQueue( 30 ) {
	memset( &curFrame, 0, sizeof(InputFrame) );
}

//! call when switched to and from this context.
void InputHandlerContext::enable( bool on ) {
	memset( &curFrame, 0, sizeof(InputFrame) );
}

//! called every frame this context is active
void InputHandlerContext::update( float fTimeInSecs ) {
	curFrame.deltaTime = fTimeInSecs;
	queueInputFrame( curFrame );
	memset( &curFrame, 0, sizeof(InputFrame) );
}

void InputHandlerContext::queueInputFrame( const InputFrame& frame ) {
	inputQueue.push_front( frame );
}

bool InputHandlerContext::dequeueInputFrame( InputFrame* frame ) {
	if( inputQueue.isNotEmpty() ) {
		inputQueue.pop_back( frame );
		return true;
	} else {
		return false;
	}
}
