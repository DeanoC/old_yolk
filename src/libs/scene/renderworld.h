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
#include "scene/screen.h"

namespace Scene {
	class Pipeline;

	typedef std::shared_ptr< std::function< void (Scene::RenderContext*) > > Renderable2DCallbackPtr;

	class RenderWorld {
	public:
		RenderWorld();
		virtual ~RenderWorld();

		virtual uint32_t addRenderable( RenderablePtr renderable );	//!< add an Renderable to the world
		virtual void removeRenderable( RenderablePtr renderable ); //!< remove this Renderable from the world
		virtual void removeRenderable( uint32_t index ); //!< remove this Renderable from the world
		virtual const RenderablePtr getRenderable( uint32_t index ) const { return renderables[index]; }
		virtual RenderablePtr getRenderable( uint32_t index ) { return renderables[index]; }

		virtual uint32_t addRenderable2D(  Renderable2DCallbackPtr _callback );
		virtual void removeRenderable2D( Renderable2DCallbackPtr _calllback ); //!< remove this Renderable2D from the world
		virtual void removeRenderable2D( uint32_t index ); //!< remove this Renderable2D from the world

		virtual void render( const ScreenPtr screen, const std::string& pipelineName, std::shared_ptr<Scene::Camera> camera, RenderContext* context );
		virtual void displayRenderResults( const ScreenPtr screen, const std::string& pipelineName, RenderContext* context );

		Core::mutex* getUpdateMutex() { return &updateMutex; }

		std::function< void (void) >		debugRenderCallback; // set a function here that will get called every frame so you can use debug_render

	protected:
		typedef tbb::concurrent_vector< RenderablePtr > RenderableContainer;
		typedef std::vector< Renderable* > 			STIndexContainer;
		typedef tbb::concurrent_vector< Renderable2DCallbackPtr > Renderable2DCallbackContainer;

		void determineVisibles( const std::shared_ptr<Scene::Camera>& camera );
		void renderRenderables( RenderContext* context, Pipeline* pipeline );

		RenderableContainer				renderables;
		Renderable2DCallbackContainer	renderables2D;

		Core::mutex 					updateMutex;
		STIndexContainer				visibleRenderables;
		std::shared_ptr<Scene::Camera>	renderCamera;
	};
}

#endif