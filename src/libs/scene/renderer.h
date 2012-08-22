#pragma once
//!-----------------------------------------------------
//!
//! \file renderer.h
//! Contains the renderer (assumes one per system)
//!
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_RENDERER_H_ )
#define YOLK_SCENE_RENDERER_H_

#if !defined( YOLK_SCENE_PIPELINE_H_ )
#include "scene/pipeline.h"
#endif
#if !defined( YOLK_SCENE_SCREEN_H_ )
#include "scene/screen.h"
#endif
#if !defined( YOLK_SCENE_RESOURCE_H_ )
#include "resource.h"
#endif

namespace Scene {
	class RenderContext;
	class DebugPrims;
	class Texture;

	//! The main singleton for the gfx subject
	class Renderer {
	public:		
		virtual ~Renderer(){};
		//! Create the screen can return nullptr if not possible
		virtual ScreenPtr createScreen( uint32_t width, uint32_t height, uint32_t flags ) = 0;
		virtual void destroyScreen( ScreenPtr screen ) = 0;

		// call once per frame to allow things to be ticked (like loaders etc.)
		virtual void houseKeep() = 0;

		size_t getNumScreens() const { return screens.size(); }
		ScreenPtr getScreen( size_t index ) const { return screens[index]; }

		RenderContext* getPrimaryContext() const { return getThreadContext(0); }
		RenderContext* getThreadContext( int index ) const { return renderContexts[index].get(); }

		void addPipeline( std::shared_ptr<Pipeline> pipeline ) {
			pipelines.push_back( pipeline );
			size_t index = pipelines.size() - 1;
			pipeline->setRendererIndex( index );
			hashPipeline[ pipeline->getName() ] = index;
		}

		size_t getNumPipelines() const { return pipelines.size(); }
		Pipeline* getPipeline( size_t index ) { return pipelines[index].get(); }
		Pipeline* getPipeline( const std::string& name ) { return pipelines[ hashPipeline[name] ].get(); }

	protected:
		std::vector<std::shared_ptr<RenderContext>>			renderContexts;	
		std::vector<std::shared_ptr<Pipeline>>				pipelines;
		std::vector<ScreenPtr>								screens;

		typedef std::unordered_map<std::string, size_t>		HashPipeline;
		HashPipeline										hashPipeline;
	};
}


#endif
