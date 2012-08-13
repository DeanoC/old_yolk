/**
 @file	Z:\Projects\wierd\source\gl\texture.h

 @brief	Declares the texture class.
 */
#pragma once
#if !defined( YOLK_SCENE_TEXTURE_H_ )
#define YOLK_SCENE_TEXTURE_H_

#include "core/resources.h"
#include "core/resourceman.h"
#include "scene/generictextureformat.h"

namespace Scene {
	//! Texture Type
	static const uint32_t TextureType = RESOURCE_NAME('T','X','T','R');

	///-------------------------------------------------------------------------------------------------
	/// \enum	TEXTURE_CREATION_FLAGS
	///
	/// \brief	Values that represent texture TEXTURE_CREATION_FLAGS. 
	///
	/// \details Default is a 2D read-only texture
	/// 		 Multisample only applies to 2D textures
	/// 		 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	enum TEXTURE_CREATION_FLAGS {
		TCF_RENDER_TARGET		= BIT(0),	//!< extra view is a render target
		TCF_MULTISAMPLE			= BIT(1),	//!< has extra samples as an MSAA texture
		TCF_PRE_FILL			= BIT(2),	//!< create a pre filled immutable texture
		TCF_CPU_READ			= BIT(3),	//!< cpu wants to map and read from the texture
		TCF_CPU_WRITE			= BIT(4),	//<! cpu wants to map and write from this texture
		TCF_GPU_WRITE_ONLY		= BIT(5),	//<! Texture is never sampled
		TCF_1D					= BIT(6),	//<! texture is 1D only
		TCF_2D					= BIT(7),	//!< the default 2D texture
		TCF_3D					= BIT(8),	//<! texture is 3D
		TCF_CUBE_MAP			= BIT(9),	//<! texture is a cube map
		TCF_ARRAY				= BIT(10),	//!< texture has arrays of slices
		TCF_COMPRESS_ON_LOAD	= BIT(11),	//!< data is uncompressed (format etc.) but compress it via GL
		TCF_BUFFER				= BIT(12),	//!< a 1D texture where the data is from a data buffer
	};

	class Texture : public Core::Resource<TextureType> {
	public:

		// when creating a texture (not loading) this is passed as the first parameter of CreateResource
		// there are platform specific versions which can be used to get closer to the metal
		struct CreationStruct {

			uint32_t				flags;			//!< various flags (including platform specific ones go here)
			GENERIC_TEXTURE_FORMAT	format;			//!< GENERIC_TEXTURE_FORMAT of the wanted texture

			uint32_t	mipLevels;					//!< how many mip levels (0=all)
			uint32_t	width;						//!< create a texture of this width
			uint32_t	height;						//!< create a texture of this height
			uint32_t	depth;						//!< 3D depth
			uint32_t 	slices;						//!< slice count

			// these properties must be entered after the constructor
			// only used with the relevant flag set
			union {
				uint32_t	sampleCount;			//!< TCF_MULTISAMPLE this is the number of samples for AA
				const void*	prefillData;			//!< TCF_PRE_FILL this is the data that will be copied into the texture
				uint32_t	bufferName;				//!< TCF_BUFFER this is the buffer used as backing
			};
			union {
				uint32_t	prefillPitch;			//!< TCF_PRE_FILL this is the pitch from line to line of the prefill data
			};
			union { 
				uint32_t	compressToFormat;		//!< format to compress to 0xFFFFFFFF for don't care
			};
		};

		struct LoadStruct {};

		Texture() :	width( 0 ), height( 0 ), depth( 0 ), slices( 0 ), mipLevels ( 0 ), sampleCount( 1 ) {}

		//! dtor
		~Texture(){};

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		uint32_t getDepth() const { return depth; }
		uint32_t getSlices() const { return slices; }
		GENERIC_TEXTURE_FORMAT getFormat() const { return format; }
		uint32_t getMipLevelCount() const { return mipLevels; }
		uint32_t getSampleCount() const { return sampleCount; }

	protected:
		uint32_t 				width;					//!< width of this texture
		uint32_t 				height;					//!< height of this texture
		uint32_t 				depth;					//!< 3D depth
		uint32_t 				slices;					//!< slice count of this texture
		GENERIC_TEXTURE_FORMAT	format;					//!< format of this texture
		uint32_t 				mipLevels;				//!< number of mip levels
		uint32_t 				sampleCount;			//!< sample count (usually 1)
	};

	typedef const Core::ResourceHandle<TextureType, Texture> TextureHandle;
	typedef TextureHandle* TextureHandlePtr;
	typedef std::shared_ptr<Texture> TexturePtr;

}

#endif