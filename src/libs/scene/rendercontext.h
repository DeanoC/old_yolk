#pragma once
//!-----------------------------------------------------
//!
//! \file rendercontext.h
//! simple redirects to the backend version
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_RENDERCONTEXT_H_)
#define YOLK_SCENE_RENDERCONTEXT_H_

#include "scene/camera.h"

namespace Scene {

	class RenderContext {
	public:
		virtual void pushDebugMarker( const char* ) const = 0;
		virtual void popDebugMarker() const = 0;		

		virtual void setCamera( const CameraPtr& _cam ) = 0;
		virtual void threadActivate() = 0; 	// must be called once, before this context is used on a particular thread
		virtual void prepToRender() = 0; // used to indicate this render context is used for rendering versus loading/non visual render stuff

		virtual void bindConstants() = 0;

	}; // sparse (largely dummy) interface to derive off to allow backends to hold context data
}

#endif // end YOLK_SCENE_RENDERCONTEXT_H_