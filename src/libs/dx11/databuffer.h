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

		// default maps entire buffer;
		void* map( Scene::RenderContext* context,  Scene::DATA_BUFFER_MAP_ACCESS access, 
						Scene::DATA_BUFFER_MAP_FLAGS flags = Scene::DBMF_NONE, 
						size_t offset = 0, size_t bytes = 0 ) override;

		void unmap( Scene::RenderContext* context ) override;

		static DataBuffer* internalCreate( const void* creation );

	protected:
		friend class DataBufferMan;
		DataBuffer( D3DResourcePtr _resource ) : Resource( _resource ) {}

	};
}

#endif