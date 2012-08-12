/**
 @file	Z:\Projects\wierd\source\gl\texture.cpp

 @brief	Implements the texture class.
 */
#include "ogl.h"
#include "core/resourceman.h"
#include "core/fileio.h"
#include "core/file_path.h"
#include "gfx.h"
#include "dds.h"
#include "texture.h"

namespace {
#include "scene/generictextureformat.h"

#define GL_COMPRESSED_RGBA_S3TC_DXT1 			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3 			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5 			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1 		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3 		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5 		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM 			GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 	GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB

#define GL_COMPRESSED_RGB_S3TC_DXT1 			GL_COMPRESSED_RGB_S3TC_DXT1_EXT 
#define GL_COMPRESSED_SRGB_S3TC_DXT1 			GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB

#define GTF_START_MACRO static uint32_t GtfToGlFormat[] = {
#define GTF_MOD_MACRO(x) GL_##x,
#define GTF_END_MACRO };
#include "scene/generictextureformat.h"

#undef GL_COMPRESSED_RGBA_S3TC_DXT1 		
#undef GL_COMPRESSED_RGBA_S3TC_DXT3 		
#undef GL_COMPRESSED_RGBA_S3TC_DXT5 		
#undef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1 	
#undef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3 	
#undef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5 	
#undef GL_COMPRESSED_RGBA_BPTC_UNORM 		
#undef GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 
#undef GL_COMPRESSED_RGB_S3TC_DXT1 		
#undef GL_COMPRESSED_SRGB_S3TC_DXT1 		
#undef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 
#undef GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT


#define TEX_IMAGE			0
#define TEX_STORAGE			1
#define TEX_STORAGE_DSA		2
// note currently TEX_IMAGE doesn't get the prefilled image data corractly
#define TEX_STORE			TEX_STORAGE_DSA

// work around for Storage bugs + not accepted by debuggers and profilers yet
void TexStorage1D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width ) {
	CORE_ASSERT( levels > 0 );
#if TEX_STORE == TEX_IMAGE
	auto pixFormat = GlFormat::getPixelFormat( fmt );
	auto pixType = GlFormat::getPixelType( fmt ); 

	glBindTexture( texType, name );
	for (GLint i = 0; i < levels; i++) {
		glTexImage1D( texType, i, fmt, width, 0,
				pixFormat, pixType, NULL);
		width = std::max( (GLint)1, width >> 1);
	}
	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE
	glBindTexture( GL_TEXTURE_1D, name );
	glTexStorage1D( texType, levels, fmt, width );
	glBindTexture( GL_TEXTURE_1D, 0 );
#elif TEX_STORE == TEX_STORAGE_DSA
	glTextureStorage1DEXT( name, texType, levels, fmt, width );
#endif
	GL_CHECK
}

void TexStorage2D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width, GLint height ) {
	CORE_ASSERT( levels > 0 );
#if TEX_STORE == TEX_IMAGE
	auto pixFormat = GlFormat::getPixelFormat( fmt );
	auto pixType = GlFormat::getPixelType( fmt ); 

	glBindTexture( texType, name );
	for (GLint i = 0; i < levels; i++) {
		glTexImage2D( texType, i, fmt, width, height, 0,
				pixFormat, pixType, NULL);
		width = std::max( (GLint)1, width >> 1);
		height = std::max( (GLint)1, height >> 1);
	}
	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE
	glBindTexture( texType, name );
	glTexStorage2D( texType, levels, fmt, width, height );
	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE_DSA
	glTextureStorage2DEXT( name, texType, levels, fmt, width, height );
#endif
	GL_CHECK
}

void TexStorage3D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width, GLint height, GLint depth ) {
	CORE_ASSERT( levels > 0 );
#if TEX_STORE == TEX_IMAGE
	auto pixFormat = GlFormat::getPixelFormat( fmt );
	auto pixType = GlFormat::getPixelType( fmt ); 

	glBindTexture( texType, name );
	for (GLint i = 0; i < levels; i++) {
		glTexImage3D( texType, i, fmt, width, height, depth, 0, 
				pixFormat, pixType, NULL);
		width = std::max( (GLint)1, width >> 1);
		height = std::max( (GLint)1, height >> 1);
	}
	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE
	glBindTexture( texType, name );
	glTexStorage3D( texType, levels, fmt, width, height, depth );
	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE_DSA
	glTextureStorage3DEXT( name, texType, levels, fmt, width, height, depth );
#endif
	GL_CHECK
}

void TexStorageBuffer( Gl::Memory::Name name, GLenum fmt,  Gl::Memory::Name bufferName ) {
	glBindTexture( GL_TEXTURE_BUFFER, name );
	glTexBuffer( GL_TEXTURE_BUFFER, fmt, bufferName );
	glBindTexture( GL_TEXTURE_BUFFER, 0 );

	GL_CHECK
}

void TexStorage2DMultisample( Gl::Memory::Name name, GLint samples, GLenum fmt, GLint width, GLint height ) {
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, name );
	glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, samples, fmt, width, height, false);
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, 0 );

}

void TexStorage3DMultisample( Gl::Memory::Name name, GLint samples, GLenum fmt, GLint width, GLint height, GLint depth ) {
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE_ARRAY, name );
	glTexImage3DMultisample( GL_TEXTURE_2D_MULTISAMPLE_ARRAY, samples, fmt, width, height, depth, false );
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0 );
}


// These Image functions must use SubImage as Storage allocated can't
// call glImageXXX as they could potentially try to resize which isn't allowed

void TexImage1D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width, uint8_t* memPtr ) {
	auto pixFmt = GlFormat::getPixelFormat( fmt );
	auto pixType = GlFormat::getPixelType( fmt ); 
	for( unsigned int i = 0; i < levels; ++i ) {
#if TEX_STORE == TEX_IMAGE ||  TEX_STORE == TEX_STORAGE
		if( i == 0 ) glBindTexture( texType, name );
		glTexSubImage1D( 	texType, i, 
							0, width, 
							pixFmt, pixType, memPtr );
		if( i == levels-1 )	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE_DSA
		glTextureSubImage1DEXT( name, texType, i, 
								0, width, 
								pixFmt, pixType, memPtr );
#endif
		memPtr += (width * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
		width = std::max<unsigned int>(1, width / 2);
	}
	GL_CHECK
}



void TexImage2D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width, GLint height, uint8_t* memPtr ) {
	auto pixFmt = GlFormat::getPixelFormat( fmt );
	auto pixType = GlFormat::getPixelType( fmt ); 
	for( unsigned int i = 0; i < levels; ++i ) {
#if TEX_STORE == TEX_IMAGE ||  TEX_STORE == TEX_STORAGE
		if( i == 0 ) glBindTexture( texType, name );
		glTexSubImage2D( 	texType, i, 
							0, 0, width, height, 						pixFmt, pixType, memPtr );
		if( i == levels-1 )	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE_DSA
		glTextureSubImage2DEXT( name, texType, i, 
								0, 0, width, height,
								pixFmt, pixType, memPtr );
#endif
		memPtr += (width * height * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
		width = std::max<unsigned int>(1, width / 2);

		// 1D Array don't mipmap slices
		if( texType != GL_TEXTURE_1D_ARRAY ) {
			height = std::max<unsigned int>(1, height / 2);
		}
	}
	GL_CHECK
}

void TexImage3D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width, GLint height, GLint depth, uint8_t* memPtr ) {
	auto pixFmt = GlFormat::getPixelFormat( fmt );
	auto pixType = GlFormat::getPixelType( fmt ); 
	for( unsigned int i = 0; i < levels; ++i ) {
#if TEX_STORE == TEX_IMAGE ||  TEX_STORE == TEX_STORAGE
		if( i == 0 ) glBindTexture( texType, name );
		glTexSubImage3D( 	texType, i,
							0, 0, 0, width, height, depth,
							pixFmt, pixType, memPtr );
		if( i == levels-1 )	glBindTexture( texType, 0 );
#elif TEX_STORE == TEX_STORAGE_DSA
		glTextureSubImage3DEXT( name, texType, i, 
								0, 0, 0, width, height, depth,
								pixFmt, pixType, memPtr );
#endif
		memPtr += (width * height * depth * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
		width = std::max<unsigned int>(1, width / 2);
		height = std::max<unsigned int>(1, height / 2);

		// only 3D texture mipmap depth as well
		if( texType == GL_TEXTURE_3D ) {
			depth = std::max<unsigned int>(1, depth / 2);
		}
	}
	GL_CHECK
}

#undef TEX_IMAGE
#undef TEX_STORAGE
#undef TEX_STORAGE_DSA
#undef TEX_STORE

}


namespace Gl {

Texture::Texture() :
	width( 0 ),
	height( 0 ),
	depth( 0 ),
	format( TF_UNKNOWN ),
	mipLevels ( 0 ),
	sampleCount( 1 ),
	renderBuffer( false ) {
}

Texture::~Texture() {
}

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

Texture* Texture::internalLoad( const Core::ResourceHandleBase* baseHandle, 
								const char* pTextureFileName, 
								bool preload ) {
	Core::FilePath path( pTextureFileName );
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

	CreationStruct cs;
	cs.format = GtfToGlFormat[ header.format ];
	cs.width = header.width;
	cs.height = header.height;
	cs.depth = header.depth;
	cs.slices = header.slices;
	cs.mipLevels = header.mipLevels;
	cs.flags = TCF_PRE_FILL;
	cs.prefillPitch = (header.width * GlFormat::getBitWidth(cs.format)) / 8;
	if( cs.depth <= 1 ) {
		if( cs.height <= 1 ) {
			cs.flags |= TCF_1D;
		} else {
			cs.flags |= TCF_2D;
		}
	} else {
		cs.flags |= TCF_3D;
	}
	if( cs.slices > 1 ) {
		cs.flags |= TCF_ARRAY;
	}
	// BIT(0) == cubemap
	if( header.flags & BIT(0) ) {
		cs.flags |= TCF_CUBE_MAP;
	}

	uint8_t* texMem = fio.takeBufferOwnership();
	cs.prefillData = (void*) Core::alignTo( (uintptr_t)texMem + sizeof( TextureFileHeader ), 8 );
//	for( int i = 0;i < header.size/4; ++i ) {
//		((uint32_t*)cs.prefillData)[i] = 0xFF00FF00; // ABGR
//	}

	Texture* tex =  internalCreate( &cs );
	CORE_DELETE_ARRAY texMem;

	return tex;
}

Texture* Texture::internalCreate( const CreationStruct* pStruct ) {
	Texture* pTexture = CORE_NEW Texture();
	pTexture->width = pStruct->width;
	pTexture->height = pStruct->height;
	pTexture->depth = pStruct->depth;
	pTexture->slices = pStruct->slices;
	pTexture->mipLevels = pStruct->mipLevels;
	pTexture->format = pStruct->format;
	Name name;
	
	GL_CHECK

	// GL create write only render target as renderbuffers
	if( (pStruct->flags & (TCF_RENDER_TARGET|TCF_GPU_WRITE_ONLY)) == (TCF_RENDER_TARGET|TCF_GPU_WRITE_ONLY) ) {
		name = pTexture->generateName( MNT_RENDER_BUFFER );
		pTexture->renderBuffer = true;

		if( pStruct->flags & TCF_MULTISAMPLE ) {
			pTexture->sampleCount = pStruct->sampleCount;
			glNamedRenderbufferStorageMultisampleEXT( name, pTexture->format, pTexture->sampleCount, pTexture->width, pTexture->height );
		} else {
			glNamedRenderbufferStorageEXT( name, pTexture->format, pTexture->width, pTexture->height );
		}
		return pTexture;
	}

	CORE_ASSERT( (pStruct->flags & TCF_GPU_WRITE_ONLY) == 0 );

	name = pTexture->generateName( MNT_TEXTURE_OBJECT ); 

	if(pStruct->flags & TCF_COMPRESS_ON_LOAD) {
		if( pStruct->compressToFormat != 0xFFFFFFFF ) {
			pTexture->format = pStruct->compressToFormat;
		} else {
			switch( GlFormat::getChannelCount( pTexture->format ) ) {
			case 4: pTexture->format = GL_COMPRESSED_RGBA; break;
			case 3: pTexture->format = GL_COMPRESSED_RGB; break;
			case 2: pTexture->format = GL_COMPRESSED_RG; break;
			case 1: pTexture->format = GL_COMPRESSED_RED; break;
			}
		}
	}

	if( pStruct->flags & TCF_MULTISAMPLE ) {
		CORE_ASSERT( (pStruct->flags & TCF_1D | pStruct->flags & TCF_3D) == 0 );
		pTexture->sampleCount = pStruct->sampleCount;
		if( pStruct->flags & TCF_ARRAY ) {
			TexStorage3DMultisample( name, pTexture->sampleCount, pTexture->format, pTexture->width, pTexture->height, pTexture->slices );
		} else {
			TexStorage2DMultisample( name, pTexture->sampleCount, pTexture->format, pTexture->width, pTexture->height );
		}
	} else {
		if( pStruct->flags & TCF_1D ) {
			if( pStruct->flags & TCF_ARRAY ) {
				TexStorage2D( GL_TEXTURE_1D_ARRAY, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->slices );
			} else {
				TexStorage1D( GL_TEXTURE_1D, pTexture->mipLevels, name, pTexture->format, pTexture->width );
			}
		} else if( pStruct->flags & (TCF_2D | TCF_CUBE_MAP) ) {
			if( pStruct->flags & TCF_CUBE_MAP ) {
				if( pStruct->flags & TCF_ARRAY ) {
					TexStorage3D( GL_TEXTURE_CUBE_MAP_ARRAY, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height, pTexture->slices );
				} else {
					TexStorage2D( GL_TEXTURE_CUBE_MAP, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height );
				}
			} else {
				if( pStruct->flags & TCF_ARRAY ) {
					TexStorage3D( GL_TEXTURE_2D_ARRAY, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height, pTexture->slices );
				} else {
					TexStorage2D( GL_TEXTURE_2D, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height );
				}
			}
		} else if( pStruct->flags & TCF_3D ) {
			CORE_ASSERT( (pStruct->flags & TCF_ARRAY) == 0 );
			TexStorage3D( GL_TEXTURE_3D, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height, pTexture->depth );
		} else if( pStruct->flags & TCF_BUFFER ) {
			TexStorageBuffer( name, pTexture->format, pStruct->bufferName );
		}
	}
	GL_CHECK;

	if( pStruct->flags & TCF_PRE_FILL ) {
		uint8_t* memPtr = (uint8_t*) pStruct->prefillData;
		uint32_t width = pStruct->width;
		uint32_t height = pStruct->height;
		uint32_t depth = pStruct->depth;
		uint32_t slices = pStruct->slices;

		// GL handles compressed format differently
		if( GlFormat::isCompressed( pStruct->format ) && 
			(pStruct->flags & TCF_COMPRESS_ON_LOAD) == false) {
				TODO_ASSERT( false && "Compress on load TODO" );
		} else {
			if( pStruct->flags & TCF_1D ) {
				if( pStruct->flags & TCF_ARRAY ) {
					TexImage2D( GL_TEXTURE_1D_ARRAY, pTexture->mipLevels, 
							name, pTexture->format, 
							width, slices, memPtr );
				} else {
					TexImage1D( GL_TEXTURE_1D, pTexture->mipLevels, 
							name, pTexture->format, 
							width, memPtr );
				}
			} else if( pStruct->flags & TCF_2D ) {
				if( pStruct->flags & TCF_ARRAY ) {
					if( (pStruct->flags & TCF_CUBE_MAP) == false ) {
						TexImage3D( GL_TEXTURE_2D_ARRAY, pTexture->mipLevels, 
								name, pTexture->format, 
								width, height, slices,
								memPtr );
					} else {
						TexImage3D( GL_TEXTURE_CUBE_MAP_ARRAY, pTexture->mipLevels, 
								name, pTexture->format, 
								width, height, slices,
								memPtr );
					}
				} else {
					if( (pStruct->flags & TCF_CUBE_MAP) == false ) {
						TexImage2D( GL_TEXTURE_2D, pTexture->mipLevels, 
								name, pTexture->format, 
								width, height, memPtr );
					} else {
						for( GLenum j = GL_TEXTURE_CUBE_MAP_POSITIVE_X; j < GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++j ) {
							TexImage2D( j, pTexture->mipLevels, 
									name, pTexture->format, 
									width, height, memPtr );
						}
					}
				}
			} else if( pStruct->flags & TCF_3D ) {
				TexImage3D( GL_TEXTURE_3D, pTexture->mipLevels, 
						name, pTexture->format, 
						width, height, depth,
						memPtr );
			}
		}
	}

	return pTexture;
}

}
