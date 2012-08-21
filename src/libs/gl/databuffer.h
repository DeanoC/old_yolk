/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( YOLK_GL_DATABUFFER_H_ )
#define YOLK_GL_DATABUFFER_H_

#include "core/resources.h"
#include "memory.h"
#include "scene/databuffer.h"

namespace Gl {

	class DataBuffer :	public Memory, public Scene::DataBuffer {
	public:	

		// default maps entire buffer;
		void* map( 	Scene::RenderContext* context, 
					Scene::DATA_BUFFER_MAP_ACCESS access, 
					Scene::DATA_BUFFER_MAP_FLAGS flags = Scene::DBMF_NONE, 
					size_t offset = 0, size_t bytes = 0 ) override;

		void unmap() override;

		static DataBuffer* internalCreate( const Core::ResourceHandleBase* baseHandle, 
											const char* name, const DataBuffer::CreationStruct* creation );

	protected:
		friend class DataBufferMan;
		DataBuffer() {}

	};
}

#endif