#ifndef YOLK_EXPORT_TEXEXPORT_H_
#define YOLK_EXPORT_TEXEXPORT_H_ 1

#if !defined( CORE_CORE_H_ )
#include "core/core.h"
#endif

#if !defined( YOLK_SCENE_GENERIC_TEXTURE_FORMAT_H_ )
#include "scene/generictextureformat.h"
#endif
#if !defined(YOLK_EXPORT_TEXTUREIMAGE_H_)
#include "textureimage.h"
#endif
namespace Export {

	// one 1 or 2D plane of a texture, from a bitmap
	struct BitmapInput {
	
		enum BI_FLAGS {
			BI_UINT8		= BIT(0),
			BI_UINT16		= BIT(1),
			BI_UINT32		= BIT(2),
			BI_HALF			= BIT(3),
			BI_FLOAT		= BIT(4),
			BI_RGBA			= BIT(5),
			BI_NORMALISED   = BIT(6),
			BI_DOUBLE		= BIT(7),
		};

		uint32_t		flags;
		int 			width;		// must always be set
		int 			height;		// 0 for 1D
		uint32_t 		channels; 	// count (>4 likely not supported)
		uint8_t const*	data;		// the oldskool data if TextureImage<> has 0 channels
		std::shared_ptr<TextureImage<double>> textureImage; // prefered storage of data
	};

	struct TextureExport {

		enum TE_FLAGS {
			TE_CUBEMAP 		= BIT( 0 ),
			TE_NORMALISED	= BIT( 1 ), // this is whether the data is normalised different from if the GTF is normalised
		};

		GENERIC_TEXTURE_FORMAT 	outFormat;
		unsigned int 			outFlags;
		int 					outWidth;
		int 					outHeight; // 0 == 1D
		int 					outDepth;  // 0 == 1D or 2D
		int 					outSlices;
		int 					outMipMapCount;

		std::vector< BitmapInput > bitmaps;
	};

	// used for texture atlases
	struct SubTexture {
		uint32_t 	index;
		float		u0,v0;
		float		u1,v1;
	};

	struct FontMetrics {
		uint16_t	dpi;
		int16_t		ascender;
		int16_t		descender;
		uint16_t	height;
		uint16_t	linegap;
	};

	// glyph used for fonts
	struct Glyph {
		uint32_t	unicode;
		uint16_t	sprite;
		uint8_t 	page;
		uint8_t		padd;
		uint16_t	width;
		uint16_t	height;
		uint16_t	offsetX;
		uint16_t	offsetY;
		uint16_t	advanceX;
		uint16_t	advanceY;
	};
}
#endif