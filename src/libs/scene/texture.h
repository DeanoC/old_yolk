/**
 @file	Z:\Projects\wierd\source\gl\texture.h

 @brief	Declares the texture class.
 */
#pragma once
#if !defined( YOLK_SCENE_TEXTURE_H_ )
#define YOLK_SCENE_TEXTURE_H_

#include "core/resources.h"
#include "core/resourceman.h"
#include "scene/resource.h"
#include "scene/generictextureformat.h"

namespace Scene {
	class RenderContext;

	//! Texture Type
	static const uint32_t TextureType = RESOURCE_NAME('T','X','T','R');

	class Texture : public Resource, public Core::Resource<TextureType> {
	public:
		friend class ResourceLoader;
		// using Resource::CreationInfo, on vc11 using doesn't work with type overloads, not sure if bug or standard but this is a workaround
		struct CreationInfo : public Resource::CreationInfo {
			CreationInfo(){};
			CreationInfo( const Resource::CreationInfo& rhs ) {
				memcpy( this, &rhs, sizeof(Resource::CreationInfo) );
			}
		};

		Texture() : width( 0 ), height( 0 ), depth( 0 ), slices( 0 ), mipLevels ( 0 ), samples( 1 ) {}

		//! dtor
		~Texture(){};

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		uint32_t getDepth() const { return depth; }
		uint32_t getSlices() const { return slices; }
		GENERIC_TEXTURE_FORMAT getFormat() const { return format; }
		uint32_t getMipLevelCount() const { return mipLevels; }
		uint32_t getSamples() const { return samples; }

		static CreationInfo TextureCtor( 	uint32_t flags, GENERIC_TEXTURE_FORMAT fmt,
											uint32_t width, uint32_t height = 1, uint32_t depth = 1, uint32_t slices = 1, 
											uint32_t mipLevels = 1, uint32_t samples = 1, 
											const void* prefillData = nullptr, uint32_t prefillPitch = 0, const void* refTex = nullptr );

	protected:
		static const void* internalPreCreate( const char* name, const CreationInfo *loader );

		GENERIC_TEXTURE_FORMAT	format;					//!< format of this texture
		uint32_t 				width;					//!< width of this texture
		uint32_t 				height;					//!< height of this texture
		uint32_t 				depth;					//!< 3D depth
		uint32_t 				slices;					//!< slice count of this texture
		uint32_t 				mipLevels;				//!< number of mip levels
		uint32_t 				samples;				//!< sample count (usually 1)
	};

	typedef const Core::ResourceHandle<TextureType, Texture> TextureHandle;
	typedef TextureHandle* TextureHandlePtr;
	typedef std::shared_ptr<Texture> TexturePtr;

	inline Texture::CreationInfo Texture::TextureCtor( 			uint32_t flags, GENERIC_TEXTURE_FORMAT fmt, 
																uint32_t width, uint32_t height, uint32_t depth, uint32_t slices, 
																uint32_t mipLevels, uint32_t samples,  
																const void* prefillData, uint32_t prefillPitch, const void* refTex ) {
		CORE_ASSERT( flags & (RCF_TEX_1D | RCF_TEX_2D | RCF_TEX_3D | RCF_TEX_CUBE_MAP) );
		CreationInfo cs;
		cs.flags = (RESOURCE_CREATION_FLAGS)flags;
		cs.width = width;
		cs.height = height;
		cs.depth = depth;
		cs.slices = slices;
		cs.mipLevels = mipLevels;
		cs.samples= samples;
		cs.format = fmt;
		cs.prefillData = prefillData;
		cs.prefillPitch = prefillPitch;
		cs.referenceTex = refTex;
		return cs;
	}

}

#endif