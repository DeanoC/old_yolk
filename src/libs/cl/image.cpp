//!-----------------------------------------------------
//!
//! \file image.cpp
//! the opencl image implementation
//!
//!-----------------------------------------------------

#include "ocl.h"
#include "gl/ogl.h"

#include "context.h"
#include "clformatcracker.h"
#include "gl/texture.h"
#include "image.h"

namespace Cl {

Image* Image::internalCreate( const Core::ResourceHandleBase* handle, const char* pName, const CreationStruct* creation ) {
	Image* img = CORE_NEW Image();
	img->type = MNT_IMAGE;

	cl_mem_flags flags = 0;
	if( (creation->flags & (ICF_KERNEL_WRITE|ICF_KERNEL_READ)) == (ICF_KERNEL_WRITE|ICF_KERNEL_READ ) ) {
		flags |= CL_MEM_READ_WRITE;
	} else {
		flags |= ( creation->flags & ICF_KERNEL_WRITE ) != 0 ? CL_MEM_WRITE_ONLY : 0;
		flags |= ( creation->flags & ICF_KERNEL_READ ) != 0 ? CL_MEM_READ_ONLY : 0;
	}
	cl_int _err;

	if( creation->flags & ICF_FROM_GL ) {
		Gl::TextureHandlePtr dbHandle = (Gl::TextureHandlePtr) creation->data;
		Gl::TexturePtr db = dbHandle->acquire();
		if( db->isRenderBuffer() ) {
			img->name = clCreateFromGLRenderbuffer(	creation->context->getContext(), 
									flags, db->getName(), &_err ); 
			CL_CHECK( "clCreateFromGLRenderbuffer", _err );
		} else {
			img->name = clCreateFromGLTexture2D(	creation->context->getContext(), 
									flags, GL_TEXTURE_2D, 0, db->getName(), &_err ); 
			CL_CHECK( "clCreateFromGLTexture2D", _err );
		}
		img->width = db->getWidth();
		img->height = db->getHeight();
		return img;
	}

	img->width = creation->width;
	img->height = creation->height;

	void* data = ( creation->flags & ICF_PREFILL ) != 0 ? creation->data : nullptr;

	// immutable are pre-filled and never changed by host or compute
	if( creation->flags & ICF_IMMUTABLE ) {
		flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;
		data = creation->data;
	} else {
		flags |= ( creation->flags & ICF_USE_HOST ) != 0 ? CL_MEM_USE_HOST_PTR : 0;
		flags |= ( creation->flags & ICF_ALLOC_HOST ) != 0 ? CL_MEM_ALLOC_HOST_PTR : 0;
		flags |= ( creation->flags & ICF_PREFILL ) != 0 ? CL_MEM_COPY_HOST_PTR : 0;
	}

	cl_image_format fmt = {
		ClFormat::getClOrder( creation->format ),
		ClFormat::getClType( creation->format )
	};
	img->name = clCreateImage2D( creation->context->getContext(), 
						flags, &fmt, img->width, img->height, 0, data, &_err ); 
	CL_CHECK( "clCreateImage2D", _err );

	return img;
}

}