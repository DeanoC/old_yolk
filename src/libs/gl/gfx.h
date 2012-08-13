#pragma once
//!-----------------------------------------------------
//!
//! \file display.h
//! Contains the main singleton for the gl subsystem
//!
//!-----------------------------------------------------
#if !defined(WIERD_GL_GFX_H)
#define WIERD_GL_GFX_H

#include "scene/renderer.h"

namespace Gl {
	class ShaderMan;
	class DebugPrims;
	class ResourceLoader;

	//! The main singleton for the gfx subject
	class Gfx : public Core::Singleton<Gfx>, public Scene::Renderer {
	public:
		//! dtor
		~Gfx();

		//! Create the screen can return nullptr if not possible
		Scene::ScreenPtr createScreen( uint32_t width, uint32_t height, uint32_t flags ) override;
		void destroyScreen() override;

		//! Present the current back buffer to the front
		void present( int backWidth, int backHeight );

		ShaderMan* getShaderMan() { return shaderMan.get(); }

	private:
		friend class Core::Singleton<Gfx>;
		//! ctor
		Gfx();

		bool createGLContext();
		void createRenderContexts();

		boost::scoped_ptr<ShaderMan>						shaderMan;
		boost::scoped_ptr<ResourceLoader>					resourceLoader;
		bool												valid;

#if PLATFORM == POSIX
		void*										x11Display;
		unsigned long								x11Window;
#else
		// platform specific 
		bool setGlPixelFormat( uint32_t flags );
#endif

	};
}


#endif
