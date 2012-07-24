#pragma once
//!-----------------------------------------------------
//!
//! \file renderworld.h
//! Holder class for an entire world as seen by the 
//! renderer
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_RENDERWORLD_H_)
#define YOLK_SCENE_RENDERWORLD_H_

#include "scene/renderable.h"
#include "scene/camera.h"

namespace Scene {

	class RenderWorld {
	public:
		RenderWorld();
		~RenderWorld();

		virtual uint32_t addRenderable( RenderablePtr renderable );	//!< add an Renderable to the world
		virtual void removeRenderable( RenderablePtr renderable ); //!< remove this Renderable from the world
		virtual void removeRenderable( uint32_t index ); //!< remove this Renderable from the world
		virtual const RenderablePtr getRenderable( uint32_t index ) const { return renderables[index]; }
		virtual RenderablePtr getRenderable( uint32_t index ) { return renderables[index]; }

		virtual uint32_t addCamera( CameraPtr camera );	//!< add an camera to the world
		virtual void removeCamera( CameraPtr camera ); //!< remove this camera from the world
		virtual void removeCamera( uint32_t index ); //!< remove this camera from the world
		virtual const CameraPtr getCamera( uint32_t index ) const { return cameras[index]; }
		virtual CameraPtr getCamera( uint32_t index ) { return cameras[index]; }

		virtual void render( RenderContext* context );

		virtual void debugDraw( RenderContext* context );

	protected:
		void renderRenderables(  RenderContext* context, const size_t pipelineIndex );

		typedef tbb::concurrent_vector< RenderablePtr > RenderableContainer;
		RenderableContainer				renderables;

		typedef tbb::concurrent_vector< CameraPtr > CameraContainer;
		CameraContainer					cameras;

	};
}

#endif