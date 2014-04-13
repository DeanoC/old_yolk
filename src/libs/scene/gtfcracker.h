#if !defined( YOLK_SCENE_GTFCRACKER_H_ )
#define YOLK_SCENE_GTFCRACKER_H_
#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	gtfformat_cracker.h
///
/// \remark	Copyright (c) 2012 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( YOLK_SCENE_GENERIC_TEXTURE_FORMAT_H_ )
#	include "generictextureformat.h"
#endif

class GtfFormat {
public:
	//! is this texture format a depth stencil format?
	static bool isDepthStencilFormat( GENERIC_TEXTURE_FORMAT fmt ) {
		switch ( fmt ) {
		case GTF_DEPTH_COMPONENT16:
		case GTF_DEPTH_COMPONENT24:
		case GTF_DEPTH_COMPONENT32F:
		case GTF_DEPTH24_STENCIL8:
		case GTF_DEPTH32F_STENCIL8:
			return true;
		default:
			return false;
		}
	}

	//! returns the number of channels per gl format
	static unsigned int getChannelCount( GENERIC_TEXTURE_FORMAT fmt ) {
		switch( fmt ) {
		case GTF_RGBA2:
		case GTF_RGBA4:
		case GTF_RGB5_A1:
		case GTF_RGBA8:
		case GTF_RGBA8I:
		case GTF_RGBA8UI:
		case GTF_RGBA8_SNORM:
		case GTF_SRGB8_ALPHA8:
		case GTF_RGB10_A2:
		case GTF_RGB10_A2UI:
		case GTF_RGBA12:
		case GTF_RGBA16:
		case GTF_RGBA16F:
		case GTF_RGBA16I:
		case GTF_RGBA16UI:
		case GTF_RGBA16_SNORM:
		case GTF_RGBA32F:
		case GTF_RGBA32I:
		case GTF_RGBA32UI:
		case GTF_COMPRESSED_RGBA_S3TC_DXT1:
		case GTF_COMPRESSED_RGBA_S3TC_DXT3:
		case GTF_COMPRESSED_RGBA_S3TC_DXT5:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT1:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT3:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT5:
		case GTF_COMPRESSED_RGBA_BPTC_UNORM:
		case GTF_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
		case GTF_COMPRESSED_RGBA:
		case GTF_COMPRESSED_SRGB_ALPHA:
			return 4;

		case GTF_R3_G3_B2:
		case GTF_RGB4:
		case GTF_RGB5:
		case GTF_RGB8:
		case GTF_RGB8I:
		case GTF_RGB8UI:
		case GTF_RGB8_SNORM:
		case GTF_SRGB8:
		case GTF_RGB9_E5:
		case GTF_R11F_G11F_B10F:
		case GTF_RGB16:
		case GTF_RGB16F:
		case GTF_RGB16I:
		case GTF_RGB16UI:
		case GTF_RGB16_SNORM:
		case GTF_RGB32F:
		case GTF_RGB32I:
		case GTF_RGB32UI:
		case GTF_COMPRESSED_RGB_S3TC_DXT1:
		case GTF_COMPRESSED_SRGB_S3TC_DXT1:
		case GTF_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		case GTF_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		case GTF_COMPRESSED_RGB:
		case GTF_COMPRESSED_SRGB:
			return 3;

		case GTF_RG8:
		case GTF_RG8I:
		case GTF_RG8UI:
		case GTF_RG8_SNORM:
		case GTF_RG16:
		case GTF_RG16F:
		case GTF_RG16I:
		case GTF_RG16UI:
		case GTF_RG16_SNORM:
		case GTF_RG32F:
		case GTF_RG32I:
		case GTF_RG32UI:
		case GTF_COMPRESSED_RG_RGTC2:
		case GTF_COMPRESSED_SIGNED_RG_RGTC2:
		case GTF_COMPRESSED_RG:
		case GTF_DEPTH24_STENCIL8:
		case GTF_DEPTH32F_STENCIL8:
			return 2;
		case GTF_R8:
		case GTF_R8I:
		case GTF_R8UI:
		case GTF_R8_SNORM:
		case GTF_R16:
		case GTF_R16F:
		case GTF_R16I:
		case GTF_R16UI:
		case GTF_R16_SNORM:
		case GTF_R32F:
		case GTF_R32I:
		case GTF_R32UI:
		case GTF_COMPRESSED_RED_RGTC1:
		case GTF_COMPRESSED_SIGNED_RED_RGTC1:
		case GTF_COMPRESSED_RED:
		case GTF_DEPTH_COMPONENT16:
		case GTF_DEPTH_COMPONENT24:
		case GTF_DEPTH_COMPONENT32F:
			return 1;
		default:
			return 0;
		}
	}

	//! Returns the number of channel bits
	static unsigned int getChannelBits( GENERIC_TEXTURE_FORMAT fmt, int channel = 0) {
		switch( fmt ) {
		case GTF_RGBA32F:
		case GTF_RGBA32I:
		case GTF_RGBA32UI:
		case GTF_RGB32F:
		case GTF_RGB32I:
		case GTF_RGB32UI:
		case GTF_RG32F:
		case GTF_RG32I:
		case GTF_RG32UI:
		case GTF_R32F:
		case GTF_R32I:
		case GTF_R32UI:
		case GTF_DEPTH_COMPONENT32F:
			return 32;
		case GTF_DEPTH32F_STENCIL8:
			if( channel == 0) return 32;
			else return 8;

		case GTF_DEPTH_COMPONENT24:
			return 24;
		case GTF_DEPTH24_STENCIL8:
			if( channel == 0) return 24;
			else return 8;
		case GTF_RGBA16:
		case GTF_RGBA16F:
		case GTF_RGBA16I:
		case GTF_RGBA16UI:
		case GTF_RGBA16_SNORM:
		case GTF_RGB16:
		case GTF_RGB16F:
		case GTF_RGB16I:
		case GTF_RGB16UI:
		case GTF_RGB16_SNORM:
		case GTF_RG16:
		case GTF_RG16F:
		case GTF_RG16I:
		case GTF_RG16UI:
		case GTF_RG16_SNORM:
		case GTF_R16:
		case GTF_R16F:
		case GTF_R16I:
		case GTF_R16UI:
		case GTF_R16_SNORM:
		case GTF_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		case GTF_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		case GTF_DEPTH_COMPONENT16:
			return 16;
		case GTF_RGBA12:
			return 12;
		case GTF_R11F_G11F_B10F:
			if( channel == 3) return 10;
			else return 11;
		case GTF_RGB10_A2:
		case GTF_RGB10_A2UI:
			if( channel == 4 ) return 2;
			else return 10;
		case GTF_RGB9_E5:
			if( channel == 4) return 5;
			else return 9;
		case GTF_RGBA8:
		case GTF_RGBA8I:
		case GTF_RGBA8UI:
		case GTF_RGBA8_SNORM:
		case GTF_SRGB8_ALPHA8:
		case GTF_RGB8:
		case GTF_RGB8I:
		case GTF_RGB8UI:
		case GTF_RGB8_SNORM:
		case GTF_SRGB8:
		case GTF_RG8:
		case GTF_RG8I:
		case GTF_RG8UI:
		case GTF_RG8_SNORM:
		case GTF_R8:
		case GTF_R8I:
		case GTF_R8UI:
		case GTF_R8_SNORM:
		case GTF_COMPRESSED_RG_RGTC2:
		case GTF_COMPRESSED_SIGNED_RG_RGTC2:
		case GTF_COMPRESSED_RED_RGTC1:
		case GTF_COMPRESSED_SIGNED_RED_RGTC1:		
			return 8;
		// BPTC_UNORM is variable between 4-7 bits for colour and 0-8 bits alpha
		case GTF_COMPRESSED_RGBA_BPTC_UNORM:
		case GTF_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
			return 6;
		case GTF_RGB5_A1:
			if( channel == 4) return 1;
			else return 5;
		case GTF_RGB5:	
		case GTF_COMPRESSED_RGBA_S3TC_DXT1:
		case GTF_COMPRESSED_RGBA_S3TC_DXT3:
		case GTF_COMPRESSED_RGBA_S3TC_DXT5:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT1:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT3:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT5:
		case GTF_COMPRESSED_RGB_S3TC_DXT1:
		case GTF_COMPRESSED_SRGB_S3TC_DXT1:
			return 5;
		case GTF_RGBA4:
		case GTF_RGB4:
			return 4;
		case GTF_R3_G3_B2:
			if( channel == 3) return 2;
			else return 3;
		case GTF_RGBA2:
			return 2;

		// unknown
		default:
		case GTF_COMPRESSED_RGBA:
		case GTF_COMPRESSED_SRGB_ALPHA:
		case GTF_COMPRESSED_RGB:
		case GTF_COMPRESSED_SRGB:
		case GTF_COMPRESSED_RG:
		case GTF_COMPRESSED_RED:
			return 0;
		}
	}

	static bool isCompressed( GENERIC_TEXTURE_FORMAT fmt ) {
		switch( fmt ) {
		case GTF_COMPRESSED_RED_RGTC1:
		case GTF_COMPRESSED_SIGNED_RED_RGTC1:
		case GTF_COMPRESSED_RED:
		case GTF_COMPRESSED_RG_RGTC2:
		case GTF_COMPRESSED_SIGNED_RG_RGTC2:
		case GTF_COMPRESSED_RG:
		case GTF_COMPRESSED_RGB_S3TC_DXT1:
		case GTF_COMPRESSED_SRGB_S3TC_DXT1:
		case GTF_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		case GTF_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		case GTF_COMPRESSED_RGB:
		case GTF_COMPRESSED_SRGB:
		case GTF_COMPRESSED_RGBA_S3TC_DXT1:
		case GTF_COMPRESSED_RGBA_S3TC_DXT3:
		case GTF_COMPRESSED_RGBA_S3TC_DXT5:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT1:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT3:
		case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT5:
		case GTF_COMPRESSED_RGBA_BPTC_UNORM:
		case GTF_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
		case GTF_COMPRESSED_RGBA:
		case GTF_COMPRESSED_SRGB_ALPHA:
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
	static unsigned int getBitWidth( GENERIC_TEXTURE_FORMAT fmt ) {
		switch( fmt ) {
			case GTF_RGBA32F:
			case GTF_RGBA32I:
			case GTF_RGBA32UI:
				return 128;

			case GTF_RGB32F:
			case GTF_RGB32I:
			case GTF_RGB32UI:
				return 96;
			case GTF_RG32F:
			case GTF_RG32I:
			case GTF_RG32UI:
			case GTF_DEPTH32F_STENCIL8:
			case GTF_RGBA16:
			case GTF_RGBA16F:
			case GTF_RGBA16I:
			case GTF_RGBA16UI:
			case GTF_RGBA16_SNORM:
				return 64;
			case GTF_RGB16:
			case GTF_RGB16F:
			case GTF_RGB16I:
			case GTF_RGB16UI:
			case GTF_RGB16_SNORM:
			case GTF_RGBA12:
				return 48;
			case GTF_RGBA8:
			case GTF_RGBA8I:
			case GTF_RGBA8UI:
			case GTF_RGBA8_SNORM:
			case GTF_SRGB8_ALPHA8:
			case GTF_RG16:
			case GTF_RG16F:
			case GTF_RG16I:
			case GTF_RG16UI:
			case GTF_RG16_SNORM:
			case GTF_R32F:
			case GTF_R32I:
			case GTF_R32UI:
			case GTF_DEPTH_COMPONENT32F:
			case GTF_DEPTH24_STENCIL8:
			case GTF_R11F_G11F_B10F:
			case GTF_RGB10_A2:
			case GTF_RGB10_A2UI:
			case GTF_RGB9_E5:
				return 32;
			case GTF_RGB8:
			case GTF_RGB8I:
			case GTF_RGB8UI:
			case GTF_RGB8_SNORM:
			case GTF_SRGB8:
			case GTF_DEPTH_COMPONENT24:
				return 24;
			case GTF_RGBA4:
			case GTF_RGB5:	
			case GTF_RGB5_A1:
			case GTF_RG8:
			case GTF_RG8I:
			case GTF_RG8UI:
			case GTF_RG8_SNORM:
			case GTF_R16:
			case GTF_R16F:
			case GTF_R16I:
			case GTF_R16UI:
			case GTF_R16_SNORM:
			case GTF_DEPTH_COMPONENT16:
			case GTF_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
			case GTF_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
				return 16;
			case GTF_RGB4:
				return 12;
			case GTF_R3_G3_B2:
			case GTF_RGBA2:
			case GTF_R8:
			case GTF_R8I:
			case GTF_R8UI:
			case GTF_R8_SNORM:
			case GTF_COMPRESSED_RG_RGTC2:
			case GTF_COMPRESSED_SIGNED_RG_RGTC2:
			case GTF_COMPRESSED_RGBA_S3TC_DXT3:
			case GTF_COMPRESSED_RGBA_S3TC_DXT5:
			case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT3:
			case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT5:
			case GTF_COMPRESSED_RGBA_BPTC_UNORM:
			case GTF_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
				return 8;
			case GTF_COMPRESSED_RGBA_S3TC_DXT1:
			case GTF_COMPRESSED_SRGB_ALPHA_S3TC_DXT1:
			case GTF_COMPRESSED_RGB_S3TC_DXT1:
			case GTF_COMPRESSED_SRGB_S3TC_DXT1:
			case GTF_COMPRESSED_RED_RGTC1:
			case GTF_COMPRESSED_SIGNED_RED_RGTC1:		
				return 4;

			// unknown
			case GTF_COMPRESSED_RGBA:
			case GTF_COMPRESSED_SRGB_ALPHA:
			case GTF_COMPRESSED_RGB:
			case GTF_COMPRESSED_SRGB:
			case GTF_COMPRESSED_RG:
			case GTF_COMPRESSED_RED:
				return 0;
			default:
				return 0;
		}
	}

	// most cases input pixel width
	static unsigned int getPixelTypeWidth( GENERIC_TEXTURE_FORMAT fmt ) {
		switch( fmt ) {
		case GTF_RGBA12: return 64;
		case GTF_RGBA2: return 32;
		case GTF_RGB4: return 32;
		case GTF_RGB5: return 16;
		default: return getBitWidth( fmt );
		}
	}
	static bool isFloat(GENERIC_TEXTURE_FORMAT fmt) {
		switch (fmt) {
		case GTF_RGBA32F:
		case GTF_RGB32F:
		case GTF_RG32F:
		case GTF_RGBA16F:
		case GTF_RGB16F:
		case GTF_RG16F:
		case GTF_R32F:
		case GTF_DEPTH_COMPONENT32F:
		case GTF_R11F_G11F_B10F:
		case GTF_R16F:
		case GTF_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		case GTF_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
			return true;
		default:
			return false;
		}
	}

	static bool isNormalised(GENERIC_TEXTURE_FORMAT fmt) {
		switch (fmt) {
		case GTF_RGBA8_SNORM:
		case GTF_RGBA16_SNORM:
		case GTF_RGB8_SNORM:
		case GTF_RGB16_SNORM:
		case GTF_RG8_SNORM:
		case GTF_RG16_SNORM:
		case GTF_R8_SNORM:
		case GTF_R16_SNORM:
		case GTF_COMPRESSED_RGBA_BPTC_UNORM:
		case GTF_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
			return true;
		default:
			return false;
		}
	}

};

#endif