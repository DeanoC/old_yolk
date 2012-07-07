/**
 @file	Z:\Projects\wierd\source\gl\texture.h

 @brief	Declares the texture class.
 */
#pragma once
#if !defined( WIERD_GL_TEXTURE_H__ )
#define WIERD_GL_TEXTURE_H__

#include "memory.h"
#include "core/resources.h"
#include "core/resourceman.h"

namespace Gl {
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

	class Texture : public Memory,
					public Core::Resource<TextureType> {
	public:
		typedef GLenum TEXTURE_FORMAT;
#define TF_UNKNOWN	-1
#define	TF_RGBA8888	GL_RGBA8
#define	TF_DXT1		GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define	TF_DXT3		GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define	TF_DXT5		GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define	TF_R8		GL_R8

		// when creating a texture (not loading) this is passed as the first parameter of CreateResource
		// there are platform specific versions which can be used to get closer to the metal
		struct CreationStruct {

			uint32_t	iFlags;					//!< various flags (including platform specific ones go here)
			TEXTURE_FORMAT	texFormat;			//!< TEXTURE_FORMAT of the wanted texture

			uint32_t	iWidth;					//!< create a texture of this width
			uint32_t	iHeight;				//!< create a texture of this height
			uint32_t	iDepth;					//!< 3D depth or array slice count
			uint32_t	iMipLevels;				//!< how many mip levels (0=all)

			// these properties must be entered after the constructor
			// only used with the relevant flag set
			union {
				uint32_t	sampleCount;			//!< TCF_MULTISAMPLE this is the number of samples for AA
				void*		prefillData;			//!< TCF_PRE_FILL this is the data that will be copied into the texture
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

		static Texture* internalLoadTexture( const Core::ResourceHandleBase* baseHandle, 
												const char* pTextureFileName, 
												bool preload );
		static Texture* internalCreateTexture( const CreationStruct* pStruct );

		Texture();
		//! dtor
		~Texture();

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		uint32_t getDepth() const { return depth; }
		TEXTURE_FORMAT getFormat() const { return format; }
		uint32_t getMipLevelCount() const { return mipLevels; }
		uint32_t getSampleCount() const { return sampleCount; }
		bool isRenderBuffer() const { return renderBuffer; }

	protected:
		uint32_t width;					//!< width of this texture
		uint32_t height;				//!< height of this texture
		uint32_t depth;					//!< depth or slice count of this texture
		TEXTURE_FORMAT format;			//!< format of this texture
		uint32_t mipLevels;				//!< number of mip levels
		uint32_t sampleCount;			//!< sample count (usually 1)
		bool	renderBuffer;			//!< true if a render buffer object
	};


	typedef const Core::ResourceHandle<TextureType, Texture> TextureHandle;
	typedef TextureHandle* TextureHandlePtr;
	typedef std::shared_ptr<Texture> TexturePtr;

}

#endif