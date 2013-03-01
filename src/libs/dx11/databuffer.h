/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( YOLK_DX11_DATABUFFER_H_ )
#define YOLK_DX11_DATABUFFER_H_

#include "core/resources.h"
#include "scene/databuffer.h"
#include "rendercontext.h"
#include "resource.h"

namespace Dx11 {

	class DataBuffer :	public Scene::DataBuffer, public Resource {
	public:	
	
		void* map( Scene::RenderContext* scontext, const Scene::RESOURCE_MAP_ACCESS _access, const int _mip = 0, const int _slice = 0, Scene::ResourceMapAccess* _outAccess = nullptr ) override {
			CORE_ASSERT( _mip == 0 );
			CORE_ASSERT( _slice == 0 );
			return Resource::map( scontext, _access, 0, _outAccess );
		}
		void unmap( Scene::RenderContext* scontext, const int _mip, const int _slice ) override {
			CORE_ASSERT( _mip == 0 );
			CORE_ASSERT( _slice == 0 );
			return Resource::unmap( scontext, 0 );
		}
		void update( Scene::RenderContext* scontext, const int _mip, const int _slice, const int _dstX, const int _dstY, const int _dstZ, 
													 const int _dstWidth, const int _dstHeight, const int _dstDepth, 
													 const Scene::ResourceMapAccess* _inAccess ) override {
			CORE_ASSERT( _mip == 0 );
			CORE_ASSERT( _slice == 0 );

			return Resource::update( scontext, 0, _dstX, _dstY, _dstZ, _dstWidth, _dstHeight, _dstDepth, _inAccess );
		}

		static DataBuffer* internalCreate( const void* creation );
		
		Scene::ViewPtr getView( uint32_t viewType ) const override { 
			return getDx11View( viewType );
		}

	protected:
		friend class DataBufferMan;
		DataBuffer( D3DResourcePtr _resource ) : Resource( _resource ) {}

	};
}

#endif