/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( WIERD_GL_DATABUFFER_H__ )
#define WIERD_GL_DATABUFFER_H__

#include "core/resources.h"
#include "memory.h"

namespace Gl {
	static const uint32_t DataBufferRType = RESOURCE_NAME('G','D','A','T');

	// data buffers can be used for more than one use, but generally have a single use,
	// this is specfied here, GENERAL is a catch all setting
	enum DATA_BUFFER_TYPE {
		DBT_VERTEX =GL_ARRAY_BUFFER,					//!< holds vertex data
		DBT_INDEX = GL_ELEMENT_ARRAY_BUFFER,			//!< holds index data
		DBT_CONSTANTS = GL_UNIFORM_BUFFER,				//!< holds shader constant data
		DBT_ATOMIC_COUNTERS = GL_ATOMIC_COUNTER_BUFFER,	//!< shader atomic counter
		DBT_TEXTURE = GL_TEXTURE_BUFFER,				//!< buffer used a texture 
		DBT_GENERAL = GL_TRANSFORM_FEEDBACK_BUFFER,		//!< multiple use or not sure etc.
	};

	enum DATA_BUFFER_CREATION_FLAGS {
		DBCF_NONE				= 0,
		DBCF_IMMUTABLE			= BIT(0),	//!< create a pre filled immutable buffer
		DBCF_CPU_UPDATES		= BIT(1),	//!< CPU updates this buffer default=regularly
		DBCF_CPU_READBACK		= BIT(2),	//!< buffer is for CPU to read back GPU data
		DBCF_GPU_COPY			= BIT(3),	//!< GPU updates buffer via copy
		DBCF_INTERMITANT		= BIT(4),	//!< updates/readbacks are irregular time wise
		DBCF_ONCE				= BIT(5),	//!< update/readbacks happens only once
	};

	class DataBuffer :	public Memory,
						public Core::Resource<DataBufferRType> {
	public:	
		static const int MIN_BUFFER_SIZE = 256; // all buffer will be increased to this size in bytes if less
		struct CreationStruct {
			DATA_BUFFER_CREATION_FLAGS	flags;	//<! how its filled, mapped, etc.
			DATA_BUFFER_TYPE			type;	//<! data type stored
			size_t						size;	//<! size in bytes
			void*						data;	//<! data to fill if PRE_FILLing
		};
		struct LoadStruct {};

		enum MAP_ACCESS {
			MA_READ_ONLY = GL_MAP_READ_BIT,
			MA_WRITE_ONLY = GL_MAP_WRITE_BIT,
			MA_READ_WRITE = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT,
		};

		enum MAP_FLAGS {
			MF_NONE = 0,
			MF_DISCARD = GL_MAP_INVALIDATE_BUFFER_BIT,
			MF_UNSYNC = GL_MAP_UNSYNCHRONIZED_BIT,
		};

		// default maps entire buffer;
		void* map( MAP_ACCESS access, MAP_FLAGS flags = MF_NONE, size_t offset = 0, size_t bytes = 0 );

		void unmap();

		size_t getSize() const { return size; }

		static DataBuffer* internalCreate( const Core::ResourceHandleBase* baseHandle, 
											const char* name, const DataBuffer::CreationStruct* creation );

	private:
		size_t				size;
		DATA_BUFFER_TYPE	type;
		friend class DataBufferMan;
		DataBuffer() {}

	};

	typedef const Core::ResourceHandle<DataBufferRType, DataBuffer> DataBufferHandle;
	typedef DataBufferHandle* DataBufferHandlePtr;
	typedef std::shared_ptr<DataBuffer> DataBufferPtr;

}

#endif