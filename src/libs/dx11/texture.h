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

		Scene::ViewPtr getView( uint32_t viewType ) const override { 
			return getDx11View( viewType );
		}

		void* map( Scene::RenderContext* scontext, const Scene::RESOURCE_MAP_ACCESS _access, const int _mip, const int _slice, Scene::ResourceMapAccess* _outAccess = nullptr ) override {
			return Resource::map( scontext, _access, (getMipLevelCount() * _slice) + _mip, _outAccess );
		}
		void unmap( Scene::RenderContext* scontext, const int _mip, const int _slice ) override {
			return Resource::unmap( scontext, (getMipLevelCount() * _slice) + _mip );
		}
		void update( Scene::RenderContext* scontext, const int _mip, const int _slice, const int _dstX, const int _dstY, const int _dstZ, 
													 const int _dstWidth, const int _dstHeight, const int _dstDepth, 
													 const Scene::ResourceMapAccess* _inAccess ) override {
			return Resource::update( scontext, (getMipLevelCount() * _slice) + _mip, _dstX, _dstY, _dstZ, _dstWidth, _dstHeight, _dstDepth, _inAccess );
		}

		DXGI_FORMAT	d3dFormat;

	};
} // end namespace Graphics

#endif // WIERD_GRAPHICS_TEXTURE_H
