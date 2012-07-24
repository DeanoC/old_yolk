//!-----------------------------------------------------
//!
//! \file textureatlas.h
//! a class that contains a number of logical textures
//! Texture atlu and sprites pages are examples of uses 
//! of this class
//!
//!-----------------------------------------------------
#if !defined(YOLK_GL_TEXTUREATLAS_H_)
#define YOLK_GL_TEXTUREATLAS_H_

#pragma once
#if !defined( YOLK_GL_TEXTURE_H_ )
#	include "texture.h"
#endif

namespace Gl {
	//! Texture Atlas Type
	static const uint32_t TextureAtlasType = RESOURCE_NAME('T','X','A','T');

	//!-----------------------------------------------------
	//!
	//! Each texture atlas contains a number of sub-textures
	//! each sub-texture has an index to its packed texture 
	//! and uv parameters
	//! A packed texture is normal texture that has several
	//! sub-textures packed into it. A texture atlas can have
	//! multiple texture to contain all its sub-textures
	//!
	//!-----------------------------------------------------
	class TextureAtlas : public Core::Resource<TextureAtlasType> {
	public:

		struct CreationStruct {};
		struct LoadStruct {};

		struct SubTexture {
			uint32_t 	index;
			float		u0,v0;
			float		u1,v1;
		};

		//! number of sub textures this atlas has
		size_t getNumberOfSubTextures() const {
			return subTextures.size();
		}
		//! number of physical packed texture this atlas has
		size_t getNumberofPackedTextures() const {
			return packedTextures.size();
		}

		//! get the handle to the specified packed texture
		TextureHandlePtr getPackedTexture( unsigned int index ) const {
			CORE_ASSERT( index < getNumberofPackedTextures() );
			return packedTextures[  index ];
		}
		//! get the subtexture structure of the specified index
		const SubTexture& getSubTexture( unsigned int index ) const {
			CORE_ASSERT( index < getNumberOfSubTextures() );
			return subTextures[ index ];
		}

		//! get the pixel dimensions of the subtexture with the specified index
		void getSubTextureDimensions( unsigned int index, unsigned int& width , unsigned int& height ) const;

		static TextureAtlas* internalLoad( 	const Core::ResourceHandleBase* baseHandle, 
											const char* pTextureAtlasFileName, 
											bool preload );

		~TextureAtlas();
	private:
		TextureAtlas(){};

		typedef std::vector<TextureHandlePtr>	PackedTextureContainer;
		typedef std::vector<SubTexture>			SubTextureContainer;

		PackedTextureContainer	packedTextures;
		SubTextureContainer		subTextures;
	};

	typedef const Core::ResourceHandle<TextureAtlasType, TextureAtlas> TextureAtlasHandle;
	typedef TextureAtlasHandle* TextureAtlasHandlePtr;
	typedef std::shared_ptr<TextureAtlas> TextureAtlasPtr;

} // end namespace Graphics

#endif // YOLK_GL_TEXTUREATLAS_H
