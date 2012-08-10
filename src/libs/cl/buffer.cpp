//!-----------------------------------------------------
//!
//! \file buffer.cpp
//! the opencl buffer implementation
//!
//!-----------------------------------------------------

#include "ocl.h"
#include "gl/ogl.h"

#include "context.h"
#include "gl/databuffer.h"
#include "buffer.h"

namespace Cl {

Buffer* Buffer::internalCreate( const Core::ResourceHandleBase* handle, const char* pName, const CreationStruct* creation ) {
	Buffer* buffer = CORE_NEW Buffer();
	buffer->type = MNT_BUFFER;

	cl_mem_flags flags = 0;
	if( (creation->flags & (BCF_KERNEL_WRITE|BCF_KERNEL_READ)) == (BCF_KERNEL_WRITE|BCF_KERNEL_READ ) ) {
		flags |= CL_MEM_READ_WRITE;
	} else {
		flags |= ( creation->flags & BCF_KERNEL_WRITE ) != 0 ? CL_MEM_WRITE_ONLY : 0;
		flags |= ( creation->flags & BCF_KERNEL_READ ) != 0 ? CL_MEM_READ_ONLY : 0;
	}
	cl_int _err;

	if( creation->flags & BCF_FROM_GL ) {
		Gl::DataBufferHandlePtr dbHandle = (Gl::DataBufferHandlePtr) creation->data;
		Gl::DataBufferPtr db = dbHandle->acquire();
		buffer->name = clCreateFromGLBuffer(	creation->context->getContext(), 
												flags, db->getName(), &_err ); 
		CL_CHECK( "clCreateFromGLBuffer", _err );
		buffer->size = db->getSize();
		return buffer;
	}

	buffer->size = creation->size;

	void* data = ( creation->flags & BCF_PREFILL ) != 0 ? creation->data : nullptr;

	// immutable are pre-filled and never changed by host or compute
	if( creation->flags & BCF_IMMUTABLE ) {
		flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;
		data = creation->data;
	} else {
		flags |= ( creation->flags & BCF_USE_HOST ) != 0 ? CL_MEM_USE_HOST_PTR : 0;
		flags |= ( creation->flags & BCF_ALLOC_HOST ) != 0 ? CL_MEM_ALLOC_HOST_PTR : 0;
		flags |= ( creation->flags & BCF_PREFILL ) != 0 ? CL_MEM_COPY_HOST_PTR : 0;
	}

	buffer->name = clCreateBuffer( creation->context->getContext(), 
									flags, creation->size, data, &_err ); 
	CL_CHECK( "clCreateBuffer", _err );

	return buffer;
}

}