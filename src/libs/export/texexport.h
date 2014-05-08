#ifndef YOLK_EXPORT_TEXEXPORT_H_
#define YOLK_EXPORT_TEXEXPORT_H_ 1
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
		};

		uint32_t		flags;
		int 			width;		// must always be set
		int 			height;		// 0 for 1D
		uint32_t 		channels; 	// count (>4 likely not supported)
		uint8_t const*	data;	  	// the actual data		
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
}
#endif