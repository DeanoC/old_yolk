#pragma once
//!-----------------------------------------------------
//!
//! \file image.h
//! Contains the cl image resource class
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_IMAGE_H)
#define WIERD_CL_IMAGE_H

#include "core/resources.h"
#include "memoryobject.h"

namespace Cl {
	class Context;
	static const uint32_t ImageRType = RESOURCE_NAME('C','I','M','G');

#if !defined(USE_OPENGL)
	// TODO dx11 path
	enum IMAGE_FORMAT {
		R8,
		R_FLOAT,
		RGBA8,
		RGBA_FLOAT,
	};
#else
	typedef GLenum	IMAGE_FORMAT;		// Gl::TEXTURE_FORMAT == Cl::IMAGE_FORMAT
#endif

	enum IMAGE_CREATION_FLAGS {
		ICF_IMMUTABLE			= BIT(0),	//!< create a pre filled immutable buffer
		ICF_KERNEL_WRITE		= BIT(1),	//!< kernel writes to this buffer 
		ICF_KERNEL_READ			= BIT(2),	//!< kernel read from this buffer (implied for IMMUTABLE)
		ICF_PREFILL				= BIT(3),	//!< copy to kernel global on create (implied for IMMUTABLE)
		ICF_ALLOC_HOST			= BIT(4),	//!< memory is in host memory space
		ICF_USE_HOST			= BIT(5),	//!< memory pointed by data is used directly MUST last as long as the buffer!
		ICF_FROM_GL				= BIT(6),	//!< CL buffer from a GL databuffer, data = DataBufferHandlePtr
	};

	class Image :	public MemoryObject,
					public Core::Resource<ImageRType> {
	public:	
		struct CreationStruct {
			Context*					context;	//<! which contex is the buffer from	
			IMAGE_CREATION_FLAGS		flags;		//<! how its filled, mapped, etc.
			IMAGE_FORMAT				format;
			size_t						width;		//<! width in pixels
			size_t						height;		//<! height in pixels
			void*						data;		//<! data to fill if PRE_FILLing
		};
		struct LoadStruct {};

		static Image* internalCreate( const Core::ResourceHandleBase* handle, const char* pName, const CreationStruct* creation );

		size_t getWidth() const { return width; }
		size_t getHeight() const { return height; }

	protected:
		size_t width;
		size_t height;
	};

	typedef const Core::ResourceHandle<ImageRType, Image> ImageHandle;
	typedef ImageHandle* ImageHandlePtr;
	typedef std::shared_ptr<Image> ImagePtr;

}

#endif