//!-----------------------------------------------------
//!
//! \file textureatlas.cpp
//! the texture atlas system
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/resourceman.h"
#include "core/file_path.h"
#include "core/fileio.h"

#include "textureatlas.h"

namespace Scene {

TextureAtlas::~TextureAtlas() {
	PackedTextureContainer::iterator texIt = packedTextures.begin();
	while( texIt != packedTextures.end() ) {
		Core::ResourceMan::get()->closeResource( *texIt );
		++texIt;
	}
}

//! Header of a Texture Atlas file	
struct TextureAtlasFileHeader {
	uint32_t magic;					//!< Should be TXAT
	uint8_t version;				//!< an incrementing version number
	uint8_t numTextures;			//!< numbers of full textures
	uint16_t numSubTextures;			//!< no of subtextures/sprites
	uint32_t size;					//!< total size
	// padding so that properties start on a 64 bit alignment 
};

const void* TextureAtlas::internalPreCreate( const char* name, const TextureAtlas::CreationInfo *loader ) {

	Core::FilePath path( name );
	path = path.ReplaceExtension( ".tat" );

	Core::MemFile fio( path.value().c_str() );
	if( !fio.isValid() ) {
		return nullptr;
	}

	TextureAtlasFileHeader header;
	fio.read( (uint8_t*) &header, sizeof(TextureAtlasFileHeader) );
	if( header.magic != TextureAtlasType ) {
		LOG(INFO) << path.value() << " is not a TXAT file\n";
		return nullptr;
	}
	if( header.version != 1 ) {
		LOG(INFO) << path.value() << " is a TXAT file of the wrong version\n";
		return nullptr;		
	}

	uint8_t* texMem = fio.takeBufferOwnership();
	uint8_t* ph = (uint8_t*) Core::alignTo( (uintptr_t)texMem + sizeof( TextureAtlasFileHeader ), 8 );
	void* startph = (void*)ph;

	TextureAtlas* ta = CORE_NEW TextureAtlas();

	uint32_t* strOff = (uint32_t*)ph;
	for( int i = 0; i < header.numTextures; ++i ) {
		TextureHandlePtr tex = TextureHandle::load( Core::fixupPointer<const char>( startph, *strOff ) );
		ta->packedTextures.push_back( tex );
		strOff += 1;
	}

	SubTexture* st = (SubTexture*)strOff;
	for( int i = 0; i < header.numSubTextures; ++i ) {
		ta->subTextures.push_back( *st );
		st += 1;
	}

	CORE_DELETE_ARRAY texMem;

	return ta;
}

void TextureAtlas::getSubTextureDimensions( unsigned int index, unsigned int& width , unsigned int& height ) const {
	const auto& sub = subTextures[ index ];
	TexturePtr texture = packedTextures[ sub.index ]->acquire();
	width = (unsigned int)( 0.5f + (sub.u1 - sub.u0) * texture->getWidth() );
	height = (unsigned int)( 0.5f + (sub.v1 - sub.v0) * texture->getHeight() );
}

}; // end namespace Graphics
