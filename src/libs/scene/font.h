//!-----------------------------------------------------
//!
//! \file font.h
//! a class that contains a number of texture atlases
//! and glyph infomation to use the sprites as a HQ font
//!
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_FONT_H_ )
#define YOLK_SCENE_FONT_H_
#pragma once

#if !defined( YOLK_SCENE_SCENE_H_ )
#include "scene.h"
#endif

#if !defined( YOLK_CORE_RESOURCEMAN_H_ )
#include "core/resourceman.h"
#endif

#if !defined( YOLK_SCENE_TEXTUREATLAS_H_ )
#	include "textureatlas.h"
#endif

namespace Scene {
	//! font Type
	static const uint32_t FontType = RESOURCE_NAME('F','O','N','T');

	//!-----------------------------------------------------
	//!
	//!
	//!-----------------------------------------------------
	class Font : public Core::Resource<FontType> {
	public:
		friend class ResourceLoader;

		struct CreationStruct {
			void*	data;	// just to pass to the second part of the loader
		};
		struct CreationInfo {};
		
		/// Glyph is stored binary on disk, don't update without revising loader/saver
		struct Glyph {
			uint32_t	unicode;
			uint16_t	sprite;
			uint8_t 	page;
			uint8_t		padd;
			uint16_t	width;	//!< in dots
			uint16_t	height;	//!< in dots
			int16_t		offsetX;	//!< in dots
			int16_t		offsetY;	//!< in dots
			int16_t		advanceX;	//!< in dots
			int16_t		advanceY;	//!< in dots

		};

		TextureAtlasHandlePtr getAtlas() const { return textureAtlas; }

		const Glyph& getGlyph(uint32_t _unicode) const;

		int getDPI() const { return header.dpi; }
		int getAscender() const { return header.ascender; }
		int getDescender() const { return header.descender; }
		int getLineGap() const { return header.height - header.descender; }// -(header.ascender + header.descender); }

		~Font();
	private:
		Font(){};

		//! Header of a Font file, stored on disk as binary, change version when changed
		struct FontFileHeader {
			uint32_t	magic;					//!< Should be FONT
			uint32_t	size;					//!< size of data block following header
			uint16_t	numRanges;				//!< no of glyph ranges
			uint8_t		version;				//!< an incrementing version number
			uint8_t		pad8;
			uint32_t	textureName;			//!< texture atlas name offset
			uint32_t	rangeOffset;			//!< where the ranges live in the data block		
			uint32_t	glyphOffset;			//!< where the ranges live in the data block

			uint16_t	dpi;					//!< dpi font was synthesised at
			int16_t		ascender;				//!< see FreeType in dots
			int16_t		descender;				//!< see FreeType in dots
			uint16_t	height;					//!< see FreeType in dots
		};

		// Range is stored binary on disk, don't update without revising loader/saver
		// a range is a continous range of glyph, max must be > min and have no holes between them
		struct Range {
			uint32_t minGlyph;				//!< min glyph for this range
			uint32_t maxGlyph;				//!< mxa glyph for this range
			uint32_t startIndex;
		};

		static const void* internalPreCreate( const char* name, const Font::CreationInfo *creation );

		uint8_t*				fontMem; // memory holding font data
		FontFileHeader			header;
		TextureAtlasHandlePtr	textureAtlas;
		const Range*			rangeTable; // pointer to base of the ranges table stored in fontMem
		const Glyph*			glyphTable; // pointer to base of the glyphs table stored in fontMem
	};

	typedef const Core::ResourceHandle<FontType, Font> FontHandle;
	typedef FontHandle* FontHandlePtr;
	typedef std::shared_ptr<Font> FontPtr;

} // end namespace Graphics

#endif // YOLK_GL_TEXTUREATLAS_H
