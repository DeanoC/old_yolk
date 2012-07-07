/**
 @file	Z:\Projects\wierd\source\gl\texture.cpp

 @brief	Implements the texture class.
 */
#include "gl.h"
#include "core/resourceman.h"
#include "gfx.h"
#include "dds.h"
#include "texture.h"

namespace {
#define TEX_IMAGE			0
#define TEX_STORAGE			1
#define TEX_STORAGE_DSA		2
#define TEX_STORE			TEX_IMAGE

// work around for Storage bugs + not accepted by debuggers and profilers yet
void TexStorage1D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width ) {
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
}

void TexStorage2D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width, GLint height ) {
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
}

void TexStorage3D( GLenum texType, GLint levels, Gl::Memory::Name name, GLenum fmt, GLint width, GLint height, GLint depth ) {
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
}

void TexStorageBuffer( Gl::Memory::Name name, GLenum fmt,  Gl::Memory::Name bufferName ) {
	glBindTexture( GL_TEXTURE_BUFFER, name );
	glTexBuffer( GL_TEXTURE_BUFFER, fmt, bufferName );
	glBindTexture( GL_TEXTURE_BUFFER, 0 );
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

Texture* Texture::internalLoadTexture( const Core::ResourceHandleBase* baseHandle, 
								const char* pTextureFileName, 
								bool preload ) {
	uint8_t* pData;
	DDS_HEADER* pHeader;
	uint8_t* pTexData;
	size_t texSize;

	bool loaded = LoadDDSFromFile( pTextureFileName, &pData, &pHeader, &pTexData, &texSize );
	if( loaded == false ) {
		CORE_DELETE_ARRAY( pData);
		return NULL;
	}

	Texture* pTexture = CORE_NEW Texture();

	CORE_DELETE_ARRAY( pData);
	return pTexture;
}

Texture* Texture::internalCreateTexture( const CreationStruct* pStruct ) {
	Texture* pTexture = CORE_NEW Texture();
	pTexture->width = pStruct->iWidth;
	pTexture->height = pStruct->iHeight;
	pTexture->depth = pStruct->iDepth;
	pTexture->mipLevels = pStruct->iMipLevels;
	pTexture->format = pStruct->texFormat;
	Name name;
	
	GL_CHECK

	// GL create write only render target as renderbuffers
	if( (pStruct->iFlags & (TCF_RENDER_TARGET|TCF_GPU_WRITE_ONLY)) == (TCF_RENDER_TARGET|TCF_GPU_WRITE_ONLY) ) {
		name = pTexture->generateName( MNT_RENDER_BUFFER );
		pTexture->renderBuffer = true;

		if( pStruct->iFlags & TCF_MULTISAMPLE ) {
			pTexture->sampleCount = pStruct->sampleCount;
			glNamedRenderbufferStorageMultisampleEXT( name, pTexture->format, pTexture->sampleCount, pTexture->width, pTexture->height );
		} else {
			glNamedRenderbufferStorageEXT( name, pTexture->format, pTexture->width, pTexture->height );
		}
		return pTexture;
	}

	CORE_ASSERT( (pStruct->iFlags & TCF_GPU_WRITE_ONLY) == 0 );
	CORE_ASSERT( (pStruct->iFlags & TCF_MULTISAMPLE) == 0 );

	name = pTexture->generateName( MNT_TEXTURE_OBJECT ); 

	if(pStruct->iFlags & TCF_COMPRESS_ON_LOAD) {
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

	if( pStruct->iFlags & TCF_1D ) {
		if( pStruct->iFlags & TCF_ARRAY ) {
			TexStorage2D( GL_TEXTURE_1D_ARRAY, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->depth );
		} else {
			TexStorage1D( GL_TEXTURE_1D, pTexture->mipLevels, name, pTexture->format, pTexture->width );
		}
	} else if( pStruct->iFlags & (TCF_2D | TCF_CUBE_MAP) ) {
		if( pStruct->iFlags & TCF_CUBE_MAP ) {
			if( pStruct->iFlags & TCF_ARRAY ) {
				TexStorage3D( GL_TEXTURE_CUBE_MAP_ARRAY, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height, pTexture->depth );
			} else {
				TexStorage2D( GL_TEXTURE_CUBE_MAP, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height );
			}
		} else {
			if( pStruct->iFlags & TCF_ARRAY ) {
				TexStorage3D( GL_TEXTURE_2D_ARRAY, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height, pTexture->depth );
			} else {
				TexStorage2D( GL_TEXTURE_2D, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height );
			}
		}
	} else if( pStruct->iFlags & TCF_3D ) {
		CORE_ASSERT( (pStruct->iFlags & TCF_ARRAY) == 0 );
		TexStorage3D( GL_TEXTURE_3D, pTexture->mipLevels, name, pTexture->format, pTexture->width, pTexture->height, pTexture->depth );
	} else if( pStruct->iFlags & TCF_BUFFER ) {
		TexStorageBuffer( name, pTexture->format, pStruct->bufferName );
	}

	GL_CHECK;

	if( pStruct->iFlags & TCF_PRE_FILL ) {
		uint8_t* memPtr = (uint8_t*) pStruct->prefillData;
		uint32_t width = pStruct->iWidth;
		uint32_t height = pStruct->iHeight;
		uint32_t depth = pStruct->iDepth;

		// GL handles compressed format differently
		if( GlFormat::isCompressed( pStruct->texFormat ) && 
			(pStruct->iFlags & TCF_COMPRESS_ON_LOAD) == false) {
				TODO_ASSERT( false && "Compress on load TODO" );
		} else {
			GLuint pixFmt = GlFormat::getPixelFormat( pStruct->texFormat );
			GLuint pixType = GlFormat::getPixelType( pStruct->texFormat );
			if( pStruct->iFlags & TCF_1D ) {
				if( pStruct->iFlags & TCF_ARRAY ) {
					for( unsigned int i = 0; i < pStruct->iMipLevels; ++i ) {
						glTextureSubImage2DEXT( name, GL_TEXTURE_1D_ARRAY, i, 
												0, 0,  width, depth,
												pixFmt, pixType, memPtr );
						memPtr += (width * depth * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
						width = std::max<unsigned int>(1, width / 2);
					}
				} else {
					for( unsigned int i = 0; i < pStruct->iMipLevels; ++i ) {
						glTextureSubImage1DEXT( name, GL_TEXTURE_1D, i, 
												0, width, 
												pixFmt, pixType, memPtr );
						memPtr += (width * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
						width = std::max<unsigned int>(1, width / 2);
					}
				}
			} else if( pStruct->iFlags & TCF_2D ) {
				if( pStruct->iFlags & TCF_ARRAY ) {
					if( pStruct->iFlags & TCF_CUBE_MAP ) {
						depth = depth * 6;
					}
					for( unsigned int i = 0; i < pStruct->iMipLevels; ++i ) {
						glTextureSubImage3DEXT( name, GL_TEXTURE_2D_ARRAY, i, 
												0, 0, 0, width, height, depth,
												pixFmt, pixType, memPtr );
						memPtr += (width * height * depth * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
						width = std::max<unsigned int>(1, width / 2);
						height = std::max<unsigned int>(1, height / 2);
						depth = std::max<unsigned int>(1, depth / 2);
					}
				} else {
					if( (pStruct->iFlags & TCF_CUBE_MAP) == false ) {
						for( unsigned int i = 0; i < pStruct->iMipLevels; ++i ) {
							glTextureSubImage2DEXT( name, GL_TEXTURE_2D, i, 
													0, 0, width, height,
													pixFmt, pixType, memPtr );
							memPtr += (width * height * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
							width = std::max<unsigned int>(1, width / 2);
							height = std::max<unsigned int>(1, height / 2);
						}
					} else {
						for( GLenum j = GL_TEXTURE_CUBE_MAP_POSITIVE_X; j < GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++ j ) {
							for( unsigned int i = 0; i < pStruct->iMipLevels; ++i ) {
								glTextureSubImage2DEXT( name, j, i, 
														0, 0, width, height,
														pixFmt, pixType, memPtr );
								memPtr += (width * height * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
								width = std::max<unsigned int>(1, width / 2);
								height = std::max<unsigned int>(1, height / 2);
							}
						}
					}
				}
			} else if( pStruct->iFlags & TCF_3D ) {
				for( unsigned int i = 0; i < pStruct->iMipLevels; ++i ) {
					glTextureSubImage3DEXT( name, GL_TEXTURE_3D, i, 
											0, 0, 0, width, height, depth,
											pixFmt, pixType, memPtr );
					memPtr += (width * height * depth * GlFormat::getPixelTypeWidth(pixFmt)) / 8;
					width = std::max<unsigned int>(1, width / 2);
					height = std::max<unsigned int>(1, height / 2);
					depth = std::max<unsigned int>(1, depth / 2);
				}
			}
		}
	}

	return pTexture;
}

}