#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	gl\giformat_cracker.h
///
/// \brief	Declares the dxgiformat cracker class.
///
/// \details	
///		dxgiformat_cracker description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( YOLK_GL_CRACKER_H__ )
#define YOLK_GL_CRACKER_H__

class GlFormat {
public:
	//! is this texture format a depth stencil format?
	static bool isDepthStencilFormat( GLuint fmt ) {
		switch ( fmt ) {
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
		case GL_DEPTH_COMPONENT32F:
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			return true;
		default:
			return false;
		}
	}

	//! returns the number of channels per dxgi format
	static unsigned int getChannelCount( GLuint fmt ) {
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
			return 4;

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
			return 3;

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
			return 2;
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
			return 1;
		default:
			return 0;
		}
	}

	//! Returns the number of color channel bits in the specified DXGI_FORMAT
	static unsigned int getColorChannelBits( GLuint fmt ) {
		switch( fmt ) {
		case GL_RGBA32F:
		case GL_RGBA32I:
		case GL_RGBA32UI:
		case GL_RGB32F:
		case GL_RGB32I:
		case GL_RGB32UI:
		case GL_RG32F:
		case GL_RG32I:
		case GL_RG32UI:
		case GL_R32F:
		case GL_R32I:
		case GL_R32UI:
		case GL_DEPTH32F_STENCIL8:
		case GL_DEPTH_COMPONENT32F:
		case GL_DEPTH_COMPONENT32:
			return 32;

		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH_COMPONENT24:
			return 24;

		case GL_RGBA16:
		case GL_RGBA16F:
		case GL_RGBA16I:
		case GL_RGBA16UI:
		case GL_RGBA16_SNORM:
		case GL_RGB16:
		case GL_RGB16F:
		case GL_RGB16I:
		case GL_RGB16UI:
		case GL_RGB16_SNORM:
		case GL_RG16:
		case GL_RG16F:
		case GL_RG16I:
		case GL_RG16UI:
		case GL_RG16_SNORM:
		case GL_R16:
		case GL_R16F:
		case GL_R16I:
		case GL_R16UI:
		case GL_R16_SNORM:
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
		case GL_DEPTH_COMPONENT16:
			return 16;
		case GL_RGBA12:
			return 12;
		case GL_R11F_G11F_B10F:
			return 11;		
		case GL_RGB10_A2:
		case GL_RGB10_A2UI:
			return 10;
		case GL_RGB9_E5:
			return 9;

		case GL_RGBA8:
		case GL_RGBA8I:
		case GL_RGBA8UI:
		case GL_RGBA8_SNORM:
		case GL_SRGB8_ALPHA8:
		case GL_RGB8:
		case GL_RGB8I:
		case GL_RGB8UI:
		case GL_RGB8_SNORM:
		case GL_SRGB8:
		case GL_RG8:
		case GL_RG8I:
		case GL_RG8UI:
		case GL_RG8_SNORM:
		case GL_R8:
		case GL_R8I:
		case GL_R8UI:
		case GL_R8_SNORM:
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
		case GL_COMPRESSED_RED_RGTC1:
		case GL_COMPRESSED_SIGNED_RED_RGTC1:		
			return 8;
		// BPTC_UNORM is variable between 4-7 bits for colour and 0-8 bits alpha
		case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
			return 6;
		case GL_RGB5:	
		case GL_RGB5_A1:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
			return 5;
		case GL_RGBA4:
		case GL_RGB4:
			return 4;
		case GL_R3_G3_B2:
			return 3;		
		case GL_RGBA2:
			return 2;

		// unknown
		default:
		case GL_COMPRESSED_RGBA:
		case GL_COMPRESSED_SRGB_ALPHA:
		case GL_COMPRESSED_RGB:
		case GL_COMPRESSED_SRGB:
		case GL_COMPRESSED_RG:
		case GL_COMPRESSED_RED:
			return 0;
		}
	}

	static bool isCompressed( GLuint fmt ) {
		switch( fmt ) {
		case GL_COMPRESSED_RED_RGTC1:
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
		case GL_COMPRESSED_RED:
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
		case GL_COMPRESSED_RG:
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB:
		case GL_COMPRESSED_SRGB:
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
			return true;
		default:
			return false;
		}
	}


	//! Returns the number of bits in the specified DXGI_FORMAT-----------------------------------------
	/// \fn	static unsigned int getBitWidth( DXGI_FORMAT fmt )
	///
	/// \brief	Returns the number of bits in the specified DXGI_FORMAT
	///
	/// \details BCx are block compressed the value is the amount per texel even tho never accessed
	/// 		 like that so BC1 take 4 bits to store a RGBA5551 but can't be accessed 1 pixel at
	/// 		 at time
	///
	/// \param	fmt	Describes the format to use. 
	///
	/// \return	The bit width of the format.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static unsigned int getBitWidth( GLuint fmt ) {
		switch( fmt ) {
			case GL_RGBA32F:
			case GL_RGBA32I:
			case GL_RGBA32UI:
				return 128;

			case GL_RGB32F:
			case GL_RGB32I:
			case GL_RGB32UI:
				return 96;
			case GL_RG32F:
			case GL_RG32I:
			case GL_RG32UI:
			case GL_DEPTH32F_STENCIL8:
			case GL_RGBA16:
			case GL_RGBA16F:
			case GL_RGBA16I:
			case GL_RGBA16UI:
			case GL_RGBA16_SNORM:
				return 64;
			case GL_RGB16:
			case GL_RGB16F:
			case GL_RGB16I:
			case GL_RGB16UI:
			case GL_RGB16_SNORM:
			case GL_RGBA12:
				return 48;
			case GL_RGBA8:
			case GL_RGBA8I:
			case GL_RGBA8UI:
			case GL_RGBA8_SNORM:
			case GL_SRGB8_ALPHA8:
			case GL_RG16:
			case GL_RG16F:
			case GL_RG16I:
			case GL_RG16UI:
			case GL_RG16_SNORM:
			case GL_R32F:
			case GL_R32I:
			case GL_R32UI:
			case GL_DEPTH_COMPONENT32F:
			case GL_DEPTH_COMPONENT32:
			case GL_DEPTH24_STENCIL8:
			case GL_R11F_G11F_B10F:
			case GL_RGB10_A2:
			case GL_RGB10_A2UI:
			case GL_RGB9_E5:
				return 32;
			case GL_RGB8:
			case GL_RGB8I:
			case GL_RGB8UI:
			case GL_RGB8_SNORM:
			case GL_SRGB8:
			case GL_DEPTH_COMPONENT24:
				return 24;
			case GL_RGBA4:
			case GL_RGB5:	
			case GL_RGB5_A1:
			case GL_RG8:
			case GL_RG8I:
			case GL_RG8UI:
			case GL_RG8_SNORM:
			case GL_R16:
			case GL_R16F:
			case GL_R16I:
			case GL_R16UI:
			case GL_R16_SNORM:
			case GL_DEPTH_COMPONENT16:
			case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
			case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
				return 16;
			case GL_RGB4:
				return 12;
			case GL_R3_G3_B2:
			case GL_RGBA2:
			case GL_R8:
			case GL_R8I:
			case GL_R8UI:
			case GL_R8_SNORM:
			case GL_COMPRESSED_RG_RGTC2:
			case GL_COMPRESSED_SIGNED_RG_RGTC2:
			case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
			case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
			case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
				return 8;
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RED_RGTC1:
			case GL_COMPRESSED_SIGNED_RED_RGTC1:		
				return 4;

			// unknown
			case GL_COMPRESSED_RGBA:
			case GL_COMPRESSED_SRGB_ALPHA:
			case GL_COMPRESSED_RGB:
			case GL_COMPRESSED_SRGB:
			case GL_COMPRESSED_RG:
			case GL_COMPRESSED_RED:
				return 0;
			default:
				return 0;
		}
	}

	static GLuint getPixelFormat( GLuint fmt ) {
		switch( fmt ) {
		case GL_RGBA2:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGBA8:
		case GL_SRGB8_ALPHA8:
		case GL_RGB10_A2:
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGBA16F:
		case GL_RGBA32F:
			return GL_RGBA;
		case GL_RGBA8I:
		case GL_RGBA8UI:
		case GL_RGBA8_SNORM:
		case GL_RGB10_A2UI:
		case GL_RGBA16I:
		case GL_RGBA16UI:
		case GL_RGBA16_SNORM:
		case GL_RGBA32I:
		case GL_RGBA32UI:
			return GL_RGBA_INTEGER;
		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB8:
		case GL_SRGB8:
		case GL_RGB8_SNORM:
		case GL_RGB9_E5:
		case GL_R11F_G11F_B10F:
		case GL_RGB16:
		case GL_RGB16_SNORM:
		case GL_RGB16F:
		case GL_RGB32F:
			return GL_RGB;
		case GL_RGB8I:
		case GL_RGB8UI:
		case GL_RGB16I:
		case GL_RGB16UI:
		case GL_RGB32I:
		case GL_RGB32UI:
			return GL_RGB_INTEGER;
		case GL_RG8:
		case GL_RG8_SNORM:
		case GL_RG16:
		case GL_RG16_SNORM:
		case GL_RG16F:
		case GL_RG32F:
			return GL_RG;

		case GL_RG8I:
		case GL_RG8UI:
		case GL_RG16I:
		case GL_RG16UI:
		case GL_RG32I:
		case GL_RG32UI:
			return GL_RG_INTEGER;
		case GL_R8:
		case GL_R8_SNORM:
		case GL_R16:
		case GL_R16_SNORM:
		case GL_R16F:
		case GL_R32F:
			return GL_RED;
		case GL_R8I:
		case GL_R8UI:
		case GL_R16I:
		case GL_R16UI:
		case GL_R32I:
		case GL_R32UI:
			return GL_RED_INTEGER;
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
		case GL_DEPTH_COMPONENT32F:
			return GL_DEPTH_COMPONENT;
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			return GL_DEPTH_STENCIL;

		case GL_COMPRESSED_RED_RGTC1:
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
		case GL_COMPRESSED_RED:
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
		case GL_COMPRESSED_RG:
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB:
		case GL_COMPRESSED_SRGB:
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
			return fmt;
		default:
			return 0;
		}
	}
	static GLuint getPixelType( GLuint fmt ) {
		switch( fmt ) {
		case GL_RGBA8:
		case GL_SRGB8_ALPHA8:
		case GL_RGBA8UI:
		case GL_RGB8:
		case GL_SRGB8:
		case GL_RGB8UI:
		case GL_RG8:
		case GL_RG8UI:
		case GL_R8:
		case GL_R8UI:
			return GL_UNSIGNED_BYTE;
		case GL_RGBA8I:
		case GL_RGBA8_SNORM:
		case GL_RGB8I:
		case GL_RGB8_SNORM:
		case GL_RG8I:
		case GL_RG8_SNORM:
		case GL_R8I:
		case GL_R8_SNORM:
			return GL_BYTE;
		case GL_RGBA16I:
		case GL_RGBA16_SNORM:
		case GL_RGB16I:
		case GL_RGB16_SNORM:
		case GL_RG16I:
		case GL_RG16_SNORM:
		case GL_R16I:
		case GL_R16_SNORM:
			return GL_SHORT;
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGBA16UI:
		case GL_RGB16:
		case GL_RGB16UI:
		case GL_RG16:
		case GL_RG16UI:
		case GL_R16:
		case GL_R16UI:
		case GL_DEPTH_COMPONENT16:
			return GL_UNSIGNED_SHORT;
		case GL_RGBA16F:
		case GL_RGB16F:
		case GL_RG16F:
		case GL_R16F:
			return GL_HALF_FLOAT;
		case GL_RGBA32F:
		case GL_RGB32F:
		case GL_RG32F:
		case GL_R32F:
		case GL_DEPTH_COMPONENT32F:
			return GL_FLOAT;
		case GL_RGBA32I:
		case GL_RGB32I:
		case GL_RG32I:
		case GL_R32I:
			return GL_INT;
		case GL_RGBA32UI:
		case GL_RGB32UI:
		case GL_RG32UI:
		case GL_R32UI:
		case GL_DEPTH_COMPONENT32:
			return GL_UNSIGNED_INT;

		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH24_STENCIL8:
			return GL_UNSIGNED_INT_24_8;
		case GL_DEPTH32F_STENCIL8:
			return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		case GL_RGBA2:
			return GL_UNSIGNED_BYTE;
		case GL_RGB4:
		case GL_RGBA4:
			return GL_UNSIGNED_SHORT_4_4_4_4;
		case GL_RGB5:
		case GL_RGB5_A1:
			return GL_UNSIGNED_SHORT_5_5_5_1;
		case GL_RGB10_A2:
		case GL_RGB10_A2UI:
			return GL_UNSIGNED_INT_10_10_10_2;
		case GL_R3_G3_B2:
			return GL_UNSIGNED_BYTE_3_3_2;
		case GL_RGB9_E5:
			return GL_UNSIGNED_INT_5_9_9_9_REV;
		case GL_R11F_G11F_B10F:
			return GL_UNSIGNED_INT_10F_11F_11F_REV;

		case GL_COMPRESSED_RED_RGTC1:
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
		case GL_COMPRESSED_RED:
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
		case GL_COMPRESSED_RG:
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB:
		case GL_COMPRESSED_SRGB:
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
			return fmt;
		default:
			return 0;
		}
	}

	// most cases input pixel width
	static unsigned int getPixelTypeWidth( GLuint fmt ) {
		switch( fmt ) {
		case GL_RGBA12: return 64;
		case GL_RGBA2: return 32;
		case GL_RGB4: return 32;
		case GL_RGB5: return 16;
		default: return getBitWidth( fmt );
		}
	}

};

#endif