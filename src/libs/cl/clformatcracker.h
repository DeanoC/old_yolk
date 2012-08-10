#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	cl\clformat_cracker.h
///
/// \brief	Declares the cl image format cracker class.
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( WIERD_CL_CRACKER_H__ )
#define WIERD_CL_CRACKER_H__

#if 1 //defined(USE_OPENGL)
#include "gl/ogl.h"
#endif

class ClFormat {
public:

#if 1 //defined(USE_OPENGL)
	static unsigned int getClOrder( GLuint fmt ) {
		switch( fmt ) {
		case GL_RGBA2:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGBA8:
		case GL_RGBA8I:
		case GL_RGBA8UI:
		case GL_RGBA8_SNORM:
		case GL_SRGB8_ALPHA8:
		case GL_RGB10_A2:
		case GL_RGB10_A2UI:
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGBA16F:
		case GL_RGBA16I:
		case GL_RGBA16UI:
		case GL_RGBA16_SNORM:
		case GL_RGBA32F:
		case GL_RGBA32I:
		case GL_RGBA32UI:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
		case GL_COMPRESSED_RGBA:
		case GL_COMPRESSED_SRGB_ALPHA:
			return CL_RGBA;

		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB8:
		case GL_RGB8I:
		case GL_RGB8UI:
		case GL_RGB8_SNORM:
		case GL_SRGB8:
		case GL_RGB9_E5:
		case GL_R11F_G11F_B10F:
		case GL_RGB16:
		case GL_RGB16F:
		case GL_RGB16I:
		case GL_RGB16UI:
		case GL_RGB16_SNORM:
		case GL_RGB32F:
		case GL_RGB32I:
		case GL_RGB32UI:
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB:
		case GL_COMPRESSED_SRGB:
			return CL_RGB;

		case GL_RG8:
		case GL_RG8I:
		case GL_RG8UI:
		case GL_RG8_SNORM:
		case GL_RG16:
		case GL_RG16F:
		case GL_RG16I:
		case GL_RG16UI:
		case GL_RG16_SNORM:
		case GL_RG32F:
		case GL_RG32I:
		case GL_RG32UI:
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
		case GL_COMPRESSED_RG:
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			return CL_RG;
		case GL_R8:
		case GL_R8I:
		case GL_R8UI:
		case GL_R8_SNORM:
		case GL_R16:
		case GL_R16F:
		case GL_R16I:
		case GL_R16UI:
		case GL_R16_SNORM:
		case GL_R32F:
		case GL_R32I:
		case GL_R32UI:
		case GL_COMPRESSED_RED_RGTC1:
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
		case GL_COMPRESSED_RED:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
		case GL_DEPTH_COMPONENT32F:
			return CL_R;
		default:
			return 0;
		}
	}

	static cl_channel_type getClType( GLuint fmt ) {
		switch (fmt) {
		case GL_RGB5_A1:
			return CL_UNORM_SHORT_555;

		case GL_RGBA2:
		case GL_RGBA4:
		case GL_RGBA8:
		case GL_SRGB8_ALPHA8:
		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB8:
		case GL_SRGB8:
		case GL_RG8:
		case GL_R8:
			return CL_UNORM_INT8;
		case GL_RGBA8_SNORM:
		case GL_RGB8_SNORM:
		case GL_RG8_SNORM:
		case GL_R8_SNORM:
			return CL_SNORM_INT8;
		case GL_RGBA8UI:
		case GL_RGBA16UI:
		case GL_RGBA32UI:
		case GL_RGB8UI:
		case GL_RG8UI:
		case GL_R8UI:
			return CL_UNSIGNED_INT8;
		case GL_RGBA8I:
		case GL_RGBA16I:
		case GL_RGBA32I:
		case GL_RGB8I:
		case GL_RG8I:
		case GL_R8I:
			return CL_SIGNED_INT8;

		case GL_RGB10_A2:
		case GL_RGB10_A2UI:
			return CL_UNORM_INT_101010;

		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGB16:
		case GL_RG16:
		case GL_R16:
			return CL_UNORM_INT16;
		case GL_RGBA16_SNORM:
		case GL_RGB16_SNORM:
		case GL_RG16_SNORM:
		case GL_R16_SNORM:
			return CL_SNORM_INT16;
		case GL_RGB16I:
		case GL_RGB32I:
		case GL_RG16I:
		case GL_R16I:
			return CL_SIGNED_INT16;
		case GL_RGB16UI:
		case GL_RGB32UI:
		case GL_RG16UI:
		case GL_R16UI:
			return CL_UNSIGNED_INT16;
		case GL_RGBA16F:
		case GL_RGB16F:
		case GL_RG16F:
		case GL_R16F:
			return CL_HALF_FLOAT;
		case GL_RGBA32F:
		case GL_RGB32F:
		case GL_RG32F:
		case GL_R32F:
			return CL_FLOAT;
		case GL_RG32UI:
		case GL_R32UI:
			return CL_UNSIGNED_INT32;
		case GL_RG32I:
		case GL_R32I:
			return CL_SIGNED_INT32;

		case GL_RGB9_E5:
		case GL_R11F_G11F_B10F:
		default:
			return 0;
		}
	}
#endif // end USE_OPENGL
};
#endif // end WIERL_CL_FORMATCRACKER
