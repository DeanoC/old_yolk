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

		struct Glyph {
			uint32_t	unicode;
			uint8_t 	page;
			uint8_t		padd;
			uint16_t	sprite;
		};

		TextureAtlasHandlePtr getAtlas() const { return textureAtlas; }

		const Glyph& getGlyph(uint32_t unicode) const { 
			const auto gl = glyphs.find(unicode); 
			if (gl != glyphs.end()) {
				return gl->second;
			} else {
				// if unicode code point is unavailable return codepoint 0 which should be invalid square
				return glyphs.begin()->second;
			}
		}
		~Font();
	private:
		Font(){};
		static const void* internalPreCreate( const char* name, const Font::CreationInfo *creation );

		typedef std::unordered_map<uint32_t, Glyph>		GlyphContainer;

		TextureAtlasHandlePtr	textureAtlas;
		GlyphContainer			glyphs;
	};

	typedef const Core::ResourceHandle<FontType, Font> FontHandle;
	typedef FontHandle* FontHandlePtr;
	typedef std::shared_ptr<Font> FontPtr;

} // end namespace Graphics

#endif // YOLK_GL_TEXTUREATLAS_H
