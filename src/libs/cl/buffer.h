#pragma once
//!-----------------------------------------------------
//!
//! \file buffer.h
//! Contains the cl buffer resource class
//!
//!-----------------------------------------------------
#if !defined(YOLK_CL_BUFFER_H)
#define YOLK_CL_BUFFER_H

#include "core/resources.h"
#include "memoryobject.h"

namespace Gl {
	class DataBuffer;
}

namespace Cl {
	class Context;
	static const uint32_t BufferRType = RESOURCE_NAME('C','L','B','F');

	enum BUFFER_CREATION_FLAGS {
		BCF_IMMUTABLE			= BIT(0),	//!< create a pre filled immutable buffer
		BCF_KERNEL_WRITE		= BIT(1),	//!< kernel writes to this buffer 
		BCF_KERNEL_READ			= BIT(2),	//!< kernel read from this buffer (implied for IMMUTABLE)
		BCF_PREFILL				= BIT(3),	//!< copy to kernel global on create (implied for IMMUTABLE)
		BCF_ALLOC_HOST			= BIT(4),	//!< memory is in host memory space
		BCF_USE_HOST			= BIT(5),	//!< memory pointed by data is used directly MUST last as long as the buffer!
		BCF_FROM_GL				= BIT(6),	//!< CL buffer from a GL databuffer, gldata = DataBufferHandlePtr
	};

	class Buffer :	public MemoryObject,
					public Core::Resource<BufferRType> {
	public:	
		struct CreationStruct {
			Context*					context;	//<! which contex is the buffer from	
			uint32_t					flags;		//<! how its filled, mapped, etc.
			size_t						size;		//<! size in bytes
			void*						data;		//<! data to fill if PRE_FILLing
			std::shared_ptr<Gl::DataBuffer> glbuffer;	//<! shared pointer to the gl buffer we are from if BCF_FROM_GL is set
		};
		struct LoadStruct {};

		size_t	getSize() const { return size; }

		static Buffer* internalCreate( const Core::ResourceHandleBase* handle, const char* pName, const CreationStruct* creation );
	protected:
		size_t size;
		std::shared_ptr<Gl::DataBuffer>		glDataBufferPtr;
	};

	typedef const Core::ResourceHandle<BufferRType, Buffer> BufferHandle;
	typedef BufferHandle* BufferHandlePtr;
	typedef std::shared_ptr<Buffer> BufferPtr;

}

#endif