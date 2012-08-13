#pragma once
//!-----------------------------------------------------
//!
//! \file renderer.h
//! Contains the renderer (assumes one per system)
//!
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_RENDERER_H_ )
#define YOLK_SCENE_RENDERER_H_

//#include "scene/pipeline.h"
#include "scene/screen.h"

namespace Scene {
	class RenderContext;
	class DebugPrims;
	class Pipeline;

	//! The main singleton for the gfx subject
	class Renderer {
	public:		
		enum THREAD_CONTEXT {
			RENDER_CONTEXT = 0,
			LOAD_CONTEXT
		};

		//! Create the screen can return nullptr if not possible
		virtual ScreenPtr createScreen( uint32_t width, uint32_t height, uint32_t flags ) = 0;
		//! todo take screen ptr
		virtual void destroyScreen() = 0;

		size_t getNumScreens() const { return screens.size(); }
		ScreenPtr getScreen( size_t index ) const { return screens[index]; }

		RenderContext* getThreadContext( THREAD_CONTEXT index ) const { 
			return renderContexts[index].get(); }

		size_t getNumPipelines() const { return pipelines.size(); }
		Pipeline* getPipeline( size_t index ) { return pipelines[index].get(); }

	protected:
		std::vector<std::unique_ptr<RenderContext>>			renderContexts;	
		std::vector<std::unique_ptr<Pipeline>>				pipelines;
		std::vector<ScreenPtr>								screens;

		typedef std::unordered_map<std::string, size_t>		HashPipeline;
		HashPipeline										hashPipeline;
	};
}


#endif
