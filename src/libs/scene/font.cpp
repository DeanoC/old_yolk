//!-----------------------------------------------------
//!
//! \file font.cpp
//! the font system
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/resourceman.h"
#include "core/file_path.h"
#include "core/fileio.h"
#include "textureatlas.h"
#include "font.h"

namespace Scene {

	Font::~Font() {
		Core::ResourceMan::get()->closeResource(textureAtlas);
		CORE_DELETE_ARRAY fontMem;
	}

const void* Font::internalPreCreate( const char* name, const Font::CreationInfo *loader ) {

	Core::FilePath path( name );
	path = path.ReplaceExtension( ".fnt" );

	Core::File fio( path.value().c_str() );
	if( !fio.isValid() ) {
		return nullptr;
	}
	Font* fnt = CORE_NEW Font();

	fio.read( (uint8_t*) &fnt->header, sizeof(FontFileHeader) );
	if (fnt->header.magic != FontType) {
		LOG(INFO) << path.value() << " is not a FONT file\n";
		return nullptr;
	}
	if (fnt->header.version != 2) {
		LOG(INFO) << path.value() << " is a FONT file of the wrong version\n";
		return nullptr;		
	}

	fnt->fontMem = CORE_NEW_ARRAY uint8_t[fnt->header.size];
	fio.read(fnt->fontMem, fnt->header.size);


	fnt->textureAtlas = TextureAtlasHandle::load(Core::fixupPointer<const char>(fnt->fontMem, fnt->header.textureName));
	fnt->rangeTable = Core::fixupPointer<const Range>(fnt->fontMem, fnt->header.rangeOffset);
	fnt->glyphTable = Core::fixupPointer<const Glyph>(fnt->fontMem, fnt->header.glyphOffset);

	return fnt;
}

const Font::Glyph& Font::getGlyph(uint32_t _unicode) const {
	for (uint32_t i = 0; i < header.numRanges; ++i) {
		if (_unicode >= rangeTable[i].minGlyph && _unicode < rangeTable[i].maxGlyph) {
			return glyphTable[rangeTable[i].startIndex  + (_unicode - rangeTable[i].minGlyph)];
		}
	}
	return glyphTable[0];
}

}; // end namespace Scene
