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
	}

//! Header of a Font file	
struct FontFileHeader {
	uint32_t magic;					//!< Should be FONT
	uint32_t size;					//!< total size
	uint16_t numGlyphs;				//!< no of font glyphs
	uint8_t version;				//!< an incrementing version number
	uint8_t padd8[5];	// padding so that properties start on a 64 bit alignment 
};

const void* Font::internalPreCreate( const char* name, const Font::CreationInfo *loader ) {

	Core::FilePath path( name );
	path = path.ReplaceExtension( ".fnt" );

	Core::MemFile fio( path.value().c_str() );
	if( !fio.isValid() ) {
		return nullptr;
	}

	FontFileHeader header;
	fio.read( (uint8_t*) &header, sizeof(FontFileHeader) );
	if( header.magic != FontType ) {
		LOG(INFO) << path.value() << " is not a FONT file\n";
		return nullptr;
	}
	if( header.version != 1 ) {
		LOG(INFO) << path.value() << " is a FONT file of the wrong version\n";
		return nullptr;		
	}

	uint8_t* fontMem = fio.takeBufferOwnership();
	uint8_t* ph = (uint8_t*) Core::alignTo( (uintptr_t)fontMem + sizeof( FontFileHeader ), 8 );
	void* startph = (void*)ph;

	Font* fnt = CORE_NEW Font();

	uint32_t* strOff = (uint32_t*)ph;
	fnt->textureAtlas = TextureAtlasHandle::load(Core::fixupPointer<const char>(startph, *strOff));

	Glyph* st = (Glyph*)(strOff + 1);
	for (int i = 0; i < header.numGlyphs; ++i) {
		fnt->glyphs[ st->unicode ] = (*st);
		st += 1;
	}

	CORE_DELETE_ARRAY fontMem;

	return fnt;
}

}; // end namespace Scene
