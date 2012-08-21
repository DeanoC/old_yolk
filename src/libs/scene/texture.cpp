//!-----------------------------------------------------
//!
//! \file texture.cpp
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/file_path.h"
#include "core/fileio.h"

#include "scene/gtfcracker.h"

#include "texture.h"

namespace Scene {
	
//! Header of a Texture file	
struct TextureFileHeader {
	uint32_t magic;					//!< Should be TXTR
	uint32_t version;				//!< an incrementing version number
	uint32_t format;				//!< GENERIC_TEXTURE_FORMAT
	uint32_t flags;					//!< BIT(0) == CUBEMAP
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t slices;
	uint32_t mipLevels;
	uint32_t size;					//!< total size
	// padding so that properties start on a 64 bit alignment 
};

const void* Texture::internalPreCreate( const char* name, const Texture::CreationInfo *loader ) {
	using namespace Scene;

	CreationInfo* cs = CORE_NEW CreationInfo();

	if( name == nullptr ) {
		// no name means an internal creation (the name is just for the resource cache)
		// just copy over the creation info struct into the output one
		*cs = *loader;
	} else {
		Core::FilePath path( name );
		path = path.ReplaceExtension( ".txr" );
		Core::MemFile fio( path.value().c_str() );
		if( !fio.isValid() ) {
			LOG(INFO) << path.value() << " cannot be loaded\n";
			return nullptr;
		}
	
		TextureFileHeader header;
		fio.read( (uint8_t*) &header, sizeof(TextureFileHeader) );
		if( header.magic != TextureType ) {
			LOG(INFO) << path.value() << " is not a TXTR file\n";
			return nullptr;
		}
		if( header.version != 1 ) {
			LOG(INFO) << path.value() << " is a TXTR file of the wrong version\n";
			return nullptr;		
		}

		cs->format = (GENERIC_TEXTURE_FORMAT) header.format;
		cs->width = header.width;
		cs->height = header.height;
		cs->depth = header.depth;
		cs->slices = header.slices;
		cs->mipLevels = header.mipLevels;
		cs->samples = 1;
		uint32_t csflags = RCF_ACE_IMMUTABLE | RCF_PRG_READ;
		cs->prefillPitch = (header.width * GtfFormat::getBitWidth(cs->format)) / 8;
		if( cs->depth <= 1 ) {
			if( cs->height <= 1 ) {
				csflags |= RCF_TEX_1D;
			} else {
				csflags |= RCF_TEX_2D;
			}
		} else {
			csflags |= RCF_TEX_3D;
		}
		// BIT(0) == cubemap
		if( header.flags & BIT(0) ) {
			csflags |= RCF_TEX_CUBE_MAP;
		}

		uint8_t* texMem = fio.takeBufferOwnership();
		cs->prefillData = (void*) Core::alignTo( (uintptr_t)texMem + sizeof( TextureFileHeader ), 8 );
		cs->deleteWhenFinish = texMem;
		csflags |= RCF_MISC_DELETE_FINISH;
		cs->flags = (RESOURCE_CREATION_FLAGS)csflags;

	// test filler
	//	for( int i = 0;i < header.size/4; ++i ) {
	//		((uint32_t*)cs.prefillData)[i] = 0xFF00FF00; // ABGR
	//	}
	}

	return cs;
}

}