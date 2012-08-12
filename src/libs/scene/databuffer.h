/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( YOLK_SCENE_DATABUFFER_H_ )
#define YOLK_SCENE_DATABUFFER_H_

#include "core/resources.h"

namespace Scene {
	static const uint32_t DataBufferRType = RESOURCE_NAME('G','D','A','T');

	enum DATA_BUFFER_CREATION_FLAGS {
		DBCF_NONE				= 0,
		DBCF_IMMUTABLE			= BIT(0),	//!< create a pre filled immutable buffer
		DBCF_CPU_UPDATES		= BIT(1),	//!< CPU updates this buffer default=regularly
		DBCF_CPU_READBACK		= BIT(2),	//!< buffer is for CPU to read back GPU data
		DBCF_GPU_COPY			= BIT(3),	//!< GPU updates buffer via copy
		DBCF_INTERMITANT		= BIT(4),	//!< updates/readbacks are irregular time wise
		DBCF_ONCE				= BIT(5),	//!< update/readbacks happens only once
	};

	// data buffers can be used for more than one use, but generally have a single use,
	// this is specfied here, GENERAL is a catch all setting
	enum DATA_BUFFER_TYPE {
		DBT_VERTEX,							//!< holds vertex data
		DBT_INDEX,							//!< holds index data
		DBT_CONSTANTS,						//!< holds shader constant data
		DBT_ATOMIC_COUNTERS,				//!< shader atomic counter
		DBT_TEXTURE,						//!< buffer used a texture 
		DBT_GENERAL,						//!< multiple use or not sure etc.
	};

	enum DATA_BUFFER_MAP_ACCESS {
		DBMA_READ_ONLY,
		DBMA_WRITE_ONLY,
		DBMA_READ_WRITE,
	};

	enum DATA_BUFFER_MAP_FLAGS {
		DBMF_NONE,
		DBMF_DISCARD,
		DBMF_UNSYNC,
	};

	class DataBuffer : public Core::Resource<DataBufferRType> {
	public:	
		static const int MIN_BUFFER_SIZE = 256; // all buffer will be increased to this size in bytes if less
		struct CreationStruct {
			DATA_BUFFER_CREATION_FLAGS	flags;	//<! how its filled, mapped, etc.
			DATA_BUFFER_TYPE			type;	//<! data type stored
			size_t						size;	//<! size in bytes
			const void*					data;	//<! data to fill if PRE_FILLing
		};
		struct LoadStruct {};

		// default maps entire buffer;
		virtual void* map( DATA_BUFFER_MAP_ACCESS access, DATA_BUFFER_MAP_FLAGS flags = DBMF_NONE, size_t offset = 0, size_t bytes = 0 ) = 0;

		virtual void unmap() = 0;

		size_t getSize() const { return size; }

	protected:
		size_t				size;
		DATA_BUFFER_TYPE	type;
		DataBuffer() {}

	};

	typedef const Core::ResourceHandle<DataBufferRType, DataBuffer> DataBufferHandle;
	typedef DataBufferHandle* DataBufferHandlePtr;
	typedef std::shared_ptr<DataBuffer> DataBufferPtr;
}

#endif