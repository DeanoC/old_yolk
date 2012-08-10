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

		// TODO multiple screens/views etc.
		void setActiveCamera( std::shared_ptr<Scene::Camera> cam ) { activeCamera = cam; }
		std::shared_ptr<Scene::Camera> getActiveCamera() { return activeCamera; }

		virtual void render( RenderContext* context );

		virtual void debugDraw( RenderContext* context );

		Core::mutex* getUpdateMutex() { return &updateMutex; }

	protected:
		typedef tbb::concurrent_vector< RenderablePtr > RenderableContainer;
		typedef tbb::concurrent_vector< CameraPtr > 	CameraContainer;
		typedef std::vector< uint32_t > STIndexContainer;

		void renderRenderables(  RenderContext* context, const size_t pipelineIndex );

		RenderableContainer				renderables;
		STIndexContainer				visibleRenderables;

		CameraContainer					cameras;

		std::shared_ptr<Scene::Camera>	activeCamera;
		Core::mutex 					updateMutex;
		std::shared_ptr<Scene::Camera> 	renderCamera;
		std::shared_ptr<Scene::Camera> 	debugCamera;
	};
}

#endif