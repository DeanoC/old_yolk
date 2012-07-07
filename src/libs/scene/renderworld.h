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

namespace Scene {

	class RenderWorld {
	public:
		RenderWorld();
		~RenderWorld();

		//! add an Renderable to the world
		virtual void addRenderable( RenderablePtr renderable );

		//! remove this Renderable from the world
		virtual void removeRenderable( RenderablePtr renderable );

		virtual void render( RenderContext* context );

		virtual void debugDraw( RenderContext* context );

	protected:
		void renderRenderables(  RenderContext* context, const size_t pipelineIndex );

		typedef tbb::concurrent_vector< RenderablePtr > RenderableContainer;

		RenderableContainer				renderables;
	};
}

#endif