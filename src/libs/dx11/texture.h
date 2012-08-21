//!-----------------------------------------------------
//!
//! \file texture.h
//! the engine graphics texture class
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_TEXTURE_H_ )
#define YOLK_DX11_TEXTURE_H_
#pragma once

#if !defined( YOLK_CORE_RESOURCES_H_ )
#include "core/resources.h"
#endif

#if !defined( YOLK_SCENE_TEXTURE_H_ )
#include "scene/texture.h"
#endif

#if !defined( YOLK_DX11_RESOURCE_H_ )
#include "dx11/resource.h"
#endif

namespace Dx11 {
	class RenderContext;

	class Texture : public Resource, public Scene::Texture {
	public:
		static Scene::Texture* internalCreate( const void* creation );

		//! gives you a CPU address that you can fiddle with TODO mip levels
//		void* Lock( RenderContext* context, CPU_ACCESS access, uint32_t& iOutPitch );
		//! called when you have finished with a lock
//		void Unlock( RenderContext* context );

		Texture( D3DResourcePtr _resource ) : Resource( _resource ) {}

		DXGI_FORMAT	d3dFormat;

	};
} // end namespace Graphics

#endif // WIERD_GRAPHICS_TEXTURE_H
