//!-----------------------------------------------------
//!
//! \file inputhandlercontext.cpp
//!
//!-----------------------------------------------------
#include <core/core.h>
#include <scene/camera.h>
#include <scene/rendercontext.h>
#include "clientworld.h"
#include "inputhandlercontext.h"

InputHandlerContext::InputHandlerContext( ClientWorld* _owner ) :
	owner( _owner ) {
	memset( &curFrame, 0, sizeof(InputFrame) );
}

//! call when switched to and from this context.
void InputHandlerContext::enable( bool on ) {
	memset( &curFrame, 0, sizeof(InputFrame) );
}

//! called every frame this context is active
void InputHandlerContext::update( float fTimeInSecs ) {
	curFrame.deltaTime = fTimeInSecs;
	owner->queueInputFrame( curFrame );
	memset( &curFrame, 0, sizeof(InputFrame) );
}

