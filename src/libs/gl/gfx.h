#pragma once
//!-----------------------------------------------------
//!
//! \file display.h
//! Contains the main singleton for the gl subsystem
//!
//!-----------------------------------------------------
#if !defined(WIERD_GL_GFX_H)
#define WIERD_GL_GFX_H

#include "scene/pipeline.h"

namespace Gl {
	class RenderContext;
	class ForwardPipeline;
	class ShaderMan;
	class DebugPrims;
	class ResourceLoader;
	class ImageComposer;

	//! The main singleton for the gfx subject
	class Gfx : public Core::Singleton<Gfx> {
	public:
		//! enum passed to create screen to decide what AA we want
		enum ANTI_ALIASING {
			AA_NONE = 0,
			AA_2X	= 2,
			AA_4X	= 4,
			AA_8X	= 8,
		};
		
		enum THREAD_CONTEXT {
			RENDER_CONTEXT = 0,
			LOAD_CONTEXT
		};

		//! enum that gives a idea of the maximum shader version supported by this card
		enum SHADER_MODEL {
			GL4_2,
		};

		// global system wide textures
		// remember to update the shader binder in shader.cpp if these are changed
		enum GLOBAL_TEXTURES {
			GT_DEPTH_TARGET_TEXTURE,
			GT_OLD_DEPTH_TARGET_TEXTURE,
			GT_OLD_RESULT_TARGET_TEXTURE,
			GT_SCREEN_RANDOM,
			GT_NORMAL_ENCODE,
			GT_NORMAL_DECODE,
			GT_MRT0,
			GT_MRT1,
			GT_MRT2,
			GT_MRT3,
			GT_MRT4,
			GT_MRT5,
			GT_MRT6,
			GT_MRT7,

			MAX_GLOBAL_TEXTURES
		};

		//! dtor
		~Gfx();

		//! returns the width of the current screen/framebuffer
		unsigned int getScreenWidth() const { return screenWidth; }
		//! returns the height of the current screen/framebuffer
		unsigned int getScreenHeight() const { return screenHeight; }
		ANTI_ALIASING getAntiAliasing() const { return aaSetting; }
		int getFrameCount() const { return (int) frameCount; }
		const SHADER_MODEL getShaderModel() const { return shaderModel;  }
		static const std::string& getShaderModelAsString( const SHADER_MODEL sm  );

		//! Create the visual screen, graphics ops can now comence
		bool createScreen( unsigned int iWidth, unsigned int iHeight, 
							bool bFullScreen, 
							ANTI_ALIASING aaSetting );

		//! close the screen, graphics ops won't work
		void shutdownScreen();

		//! Present the current back buffer to the front
		void present( int backWidth, int backHeight );

		//! return the defaulted texture
//		TextureHandle* getDefaultedTexture() const { return m_defaultTextureHandle; }
//		TextureHandlePtr getGlobalTexture( GLOBAL_TEXTURES texIndex ) const { 
//															return m_globalTextures[ texIndex ]; }
//		void setGlobalTexture( GLOBAL_TEXTURES texIndex, TextureHandlePtr tex) {
//															m_globalTextures[ texIndex ] = tex; }

		// returns number of primitives render this pass so far 
		uint32_t getPassPrimitiveCount() const { return passPrimitiveCount; }
		// mark a new pass from primitive count accumulator
		void resetPassPrimitiveCount(){ passPrimitiveCount = 0; }

		ShaderMan* getShaderMan() { return shaderMan.get(); }

		ImageComposer* getFinalImageComposer() { return finalComposer.get(); }

		RenderContext* getThreadRenderContext( THREAD_CONTEXT index ) const;

		void incPrimitiveCount( const uint32_t count ) {
			totalPrimitiveCount += count;
			passPrimitiveCount += count;
		}

		size_t getNumPipelines() const { return pipelines.size(); }
		Scene::Pipeline* getPipeline( size_t index ) { return pipelines[index].get(); }

	private:
		friend class Core::Singleton<Gfx>;
		//! ctor
		Gfx();

		bool createGLContext();
		void createRenderContexts();

		unsigned int		screenWidth;		//!< width in pixel of the framebuffer
		unsigned int		screenHeight;	//!< height in pixels of the framebuffer
		int					frameCount;
		ANTI_ALIASING		aaSetting;
		SHADER_MODEL							shaderModel;		//!< What is the maximum shader version supported

//		TextureHandle*		defaultTextureHandle;
//		RenderContext*		mainRenderContext;
//		TextureHandlePtr	globalTextures[ MAX_GLOBAL_TEXTURES ];
		uint32_t											totalPrimitiveCount;
		uint32_t											passPrimitiveCount;
		boost::scoped_array<RenderContext>					renderContexts;	
		std::vector<std::unique_ptr<Scene::Pipeline>>		pipelines;
		boost::scoped_ptr<ShaderMan>						shaderMan;
		boost::scoped_ptr<DebugPrims>						debugPrims;
		boost::scoped_ptr<ResourceLoader>					resourceLoader;
		boost::scoped_ptr<ImageComposer>					finalComposer;

		typedef std::unordered_map<std::string, size_t>		HashPipeline;
		HashPipeline										hashPipeline;
#if PLATFORM == POSIX
		void*										x11Display;
		unsigned long								x11Window;
#endif

	};
}


#endif
