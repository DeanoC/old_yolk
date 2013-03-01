/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( YOLK_SCENE_DATABUFFER_H_ )
#define YOLK_SCENE_DATABUFFER_H_

#include "core/resources.h"
#include "scene/resource.h"

namespace Scene {
	class RenderContext;
	static const uint32_t DataBufferType = RESOURCE_NAME('G','D','A','T');

	class DataBuffer : public Core::Resource<DataBufferType>, public Resource {
	public:	
		friend class ResourceLoader;
		static const int MIN_BUFFER_SIZE = 256; // all buffer will be increased to this size in bytes if less
		struct CreationInfo : public Scene::Resource::CreationInfo {
			CreationInfo(){};
			CreationInfo( const Scene::Resource::CreationInfo& rhs ) {
				memcpy( this, &rhs, sizeof(Scene::Resource::CreationInfo) );
			}
		};

		virtual ~DataBuffer(){};

		size_t getSize() const { return size; }
	protected:
		DataBuffer() {}
		static const void* internalPreCreate( const char* name, const CreationInfo *loader ) { return loader; };

		size_t					size;
	};

	typedef const Core::ResourceHandle<DataBufferType, DataBuffer> DataBufferHandle;
	typedef DataBufferHandle* DataBufferHandlePtr;
	typedef std::shared_ptr<DataBuffer> DataBufferPtr;
}

#endif