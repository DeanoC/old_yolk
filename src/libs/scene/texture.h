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

	class Texture : public Core::Resource<TextureType>, public Resource {
	public:
		friend class ResourceLoader;

		Texture() : width( 0 ), height( 0 ), depth( 0 ), slices( 0 ), mipLevels ( 0 ), samples( 1 ) {}

		//! dtor
		virtual ~Texture(){};

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		uint32_t getDepth() const { return depth; }
		uint32_t getSlices() const { return slices; }
		GENERIC_TEXTURE_FORMAT getFormat() const { return format; }
		uint32_t getMipLevelCount() const { return mipLevels; }
		uint32_t getSamples() const { return samples; }

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
}

#endif