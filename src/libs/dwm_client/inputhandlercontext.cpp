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

InputHandlerContext::InputHandlerContext( ClientWorld* _owner, Scene::RenderContext* _controlContext ) :
	owner( _owner ),
	controlContext( _controlContext ),
	activeCamera( nullptr ) {
	memset( &curFrame, 0, sizeof(InputFrame) );
}

//! call when switched to and from this context.
void InputHandlerContext::enable( bool on ) {
	memset( &curFrame, 0, sizeof(InputFrame) );
	if( on ) {
		activeCamera = owner->getActiveCamera();
		if( activeCamera ) {
			controlContext->setCamera( activeCamera );	
		}
	}
}

//! called every frame this context is active
void InputHandlerContext::update( float fTimeInSecs ) {
	curFrame.deltaTime = fTimeInSecs;
	owner->queueInputFrame( curFrame );
	memset( &curFrame, 0, sizeof(InputFrame) );

	if( owner->getActiveCamera() != activeCamera ) {
		activeCamera = owner->getActiveCamera();
		if( activeCamera ) {
			controlContext->setCamera( activeCamera );	
		}
	}

}

//! Context will call for your to display you debug info.
void InputHandlerContext::display() {
}

