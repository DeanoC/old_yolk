//!-----------------------------------------------------
//!
//! \file graphics.h
//! Contains the main singleton for the graphics subsystem
//!
//!-----------------------------------------------------
#if !defined(YOLK_DX11_GFX_H_)
#define YOLK_DX11_GFX_H_
#pragma once

#if !defined( YOLK_DX11_TEXTURE_H_ )
#include "dx11/texture.h"
#endif
#if !defined( YOLK_DX11_RENDERCONTEXT_H_ )
#include "dx11/rendercontext.h"
#endif

#if !defined( YOLK_SCENE_RENDERER_H_ )
#include "scene/renderer.h"
#endif

#if defined( USE_AMP )
#include "amp.h"
#endif

namespace Dx11 {

	class RenderContext;

	//! The main singleton for the gfx subject
	class Gfx : public Scene::Renderer, public Core::Singleton<Gfx> {
	public:
		~Gfx();
		Scene::ScreenPtr createScreen( uint32_t width, uint32_t height, uint32_t flags ) override;
		void destroyScreen( Scene::ScreenPtr screen ) override;
		void houseKeep() override;
		
#if defined( USE_AMP )
		concurrency::accelerator_view getAMPAcceleratorView() override;
#endif
		//! returns the actual D3D Device
		D3DDevicePtr operator()() const { return device; }
	private:
		std::vector<DXGIAdapterPtr>		adapters;
		DXGIAdapterPtr					adapter;
		D3DDevicePtr					device;
	private:
		friend class Core::Singleton<Gfx>;
		//! ctor
		Gfx();
	};
}


#endif
