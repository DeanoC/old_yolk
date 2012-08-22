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

		virtual void render( const ScreenPtr screen, const std::string& pipelineName, std::shared_ptr<Scene::Camera> camera, RenderContext* context );
		virtual void displayRenderResults( const ScreenPtr screen, const std::string& pipelineName, RenderContext* context );

		Core::mutex* getUpdateMutex() { return &updateMutex; }

	protected:
		typedef tbb::concurrent_vector< RenderablePtr > RenderableContainer;
		typedef tbb::concurrent_vector< CameraPtr > 	CameraContainer;
		typedef std::vector< Renderable* > 			STIndexContainer;

		void determineVisibles( const std::shared_ptr<Scene::Camera>& camera );
		void renderRenderables( RenderContext* context, Pipeline* pipeline );

		RenderableContainer				renderables;
		CameraContainer					cameras;


		Core::mutex 					updateMutex;
		STIndexContainer				visibleRenderables;
		std::shared_ptr<Scene::Camera>	renderCamera;
	};
}

#endif