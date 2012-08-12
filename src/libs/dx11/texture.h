//!-----------------------------------------------------
//!
//! \file texture.h
//! the engine graphics texture class
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_TEXTURE_H_ )
#define YOLK_DX11_TEXTURE_H_
#pragma once

#if !defined( YOLK_CORE_RESOURCES_H_ )
#include "core/resources.h"
#endif

#if !defined( YOLK_CORE_RESOURCEMAN_H_ )
#include "core/resourceman.h"
#endif

#if !defined( YOLK_GRAPHICS_GFXRESOURCES_H_ )
#include "gfxresources.h"
#endif

namespace Dx11 {
	class RenderContext;

	//! Texture Type
	static const uint32_t TextureType = RESOURCE_NAME('T','X','T','R');

	//! a texture resource is currently a object than can be set into a texture stage
	//! Textures can also be used as render targets.
	class Texture : public Core::Resource<TextureType> {
	public:
		typedef DXGI_FORMAT TEXTURE_FORMAT;
#define TF_UNKNOWN	DXGI_FORMAT_UNKNOWN
#define	TF_RGBA8888	DXGI_FORMAT_R8G8B8A8_UNORM
#define	TF_DXT1		DXGI_FORMAT_BC1_UNORM
#define	TF_DXT3		DXGI_FORMAT_BC2_UNORM
#define	TF_DXT5		DXGI_FORMAT_BC3_UNORM
#define	TF_R8		DXGI_FORMAT_R8_UNORM

		//! cross platform texture format, use these across all platform or just use a platform specific one
		//! each of these is defined in the platform specific code
		/*enum TEXTURE_FORMAT
			TF_UNKNOWN   = a error or none or don't care code 
			TF_RGBA8888  = 8 bit unsigned RGBA in shader 0 to 1
			TF_DXT1		 = AKA S3TC1 4 bit per pixel block compressed RGB
			TF_DXT3		 = AKA S3TC3 4 bit per pixel block compression RGB + 4 bit interpolate A (8 bit total)
			TF_DXT5		 = AKA S3TC5 4 bit per pixel block compression RGB + 4 bit discrete A (8 bit total)
			TF_R8		 = 8 bit unsigned R in shader 0 to 1
		*/

		// when creating a texture (not loading) this is passed as the first parameter of CreateResource
		// there are platform specific versions which can be used to get closer to the metal
		struct CreationStruct {
			CreationStruct( uint32_t _flags, uint32_t _width, 
							uint32_t _height = 1, TEXTURE_FORMAT _format = TF_UNKNOWN, 
							uint32_t _mips = 1, 
							uintptr_t _param0 = 0, uintptr_t _param1 = 0, uintptr_t _param2 = 0 ) :
				iFlags( _flags ), iWidth( _width ), iHeight( _height ), texFormat( _format ), iMipLevels( _mips ), 
				platformParam0( _param0 ), platformParam1( _param1 ), platformParam2( _param2 ) {};
			CreationStruct( uint32_t _flags ) : iFlags( _flags ) {}

			uint32_t	iFlags;					//!< various flags (including platform specific ones go here)

			uint32_t	iWidth;					//!< create a texture of this width
			uint32_t	iHeight;				//!< create a texture of this height

			TEXTURE_FORMAT	texFormat;			//!< TEXTURE_FORMAT of the wanted texture
			uint32_t	iMipLevels;				//!< how many mip levels (0=all)

			union {
				uintptr_t	platformParam0;			//!< Used to pass some platform specific (defined by iFlags) through
				void*		fromD3DTex;				//!< CF_FROM_D3D_TEXTURE this is the D3D texture to wrap
				uint32_t	sampleCount;			//!< CF_MULTISAMPLE this is the number of samples for AA
				uint32_t	iDepth;					//!< 3D depth or depth for array slices
				uint32_t	structureSize;			//!< size of a structure for structured buffers
			};
			union {
				uintptr_t	platformParam1;			//!< Used to pass some platform specific (defined by iFlags) through
				void*		prefillData;			//!< CF_PRE_FILL this is the data that will be copied into the texture
			};
			union {
				uintptr_t	platformParam2;			//!< Used to pass some platform specific (defined by iFlags) through
				uint32_t	prefillPitch;			//!< CF_PRE_FILL this is the pitch from line to line of the prefill data
			};
		};
		struct LoadStruct {};

		static Texture* InternalLoadTexture( const Core::ResourceHandleBase* baseHandle, const char* pTextureFileName, bool preload );
		static Texture* InternalCreateTexture( const CreationStruct* pStruct );

		Texture();
		//! dtor
		~Texture();

		uint32_t m_iWidth;					//!< width of this texture
		uint32_t m_iHeight;					//!< height of this texture
		TEXTURE_FORMAT m_iFormat;			//!< format of this texture
		uint32_t m_iMipLevels;				//!< number of mip levels

		enum CPU_ACCESS {
			CA_READ_ONLY = D3D11_MAP_READ,	//!< Cpu map for read only purposes
			CA_WRITE_ONLY = D3D11_MAP_WRITE,	//!< Cpu map for writing to only, existing data will be there
			CA_READ_WRITE = D3D11_MAP_READ_WRITE,	//!< Cpu map for reading and writing to
			CA_WRITE_DISCARD = D3D11_MAP_WRITE_DISCARD,	//!< write only, previous data in buffer discarded
			CA_WRITE_PARTIAL = D3D11_MAP_WRITE_NO_OVERWRITE,	//!< write only, will not touch same data as previous calls
		};

		//! gives you a CPU address that you can fiddle with TODO mip levels
		void* Lock( RenderContext* context, CPU_ACCESS access, uint32_t& iOutPitch );
		//! called when you have finished with a lock
		void Unlock( RenderContext* context );
	public:
		typedef D3D11ResourceSmartPtr InternalAsyncType;

		uint32_t m_iDepth; //!< arrays and 3D texture have a depth/slice count everybody else undefined
		//! textures can come from a number of different resource types now, this is stored here
		ID3D11Resource*					m_baseTexture;
		// also a resource can have multiple views, we support two for connivance here. 
		//! Primary view is usually the normal ShaderResourceView unless not supported
		ID3D11View*						m_primaryView;
		// as we may ask for more than one extra view in the creation flags, there is an priority order
		// 1. Render target (DepthStencil or normal)
		// 2. UAV / UAV Buffer (this will be a UAV Buffer view or a UAV texture view)
		// 3. Buffer
		// To get a view of the other, manually create the view
		ID3D11View*						m_extraView;


		///-------------------------------------------------------------------------------------------------
		/// \enum	CREATION_FLAGS
		///
		/// \brief	Values that represent texture CREATION_FLAGS. 
		///
		/// \details PC textures/resources can be very complex in Dx11+, this attempts to support most
		/// 		 in a simple api. Not all combinations are supported and some also required others
		/// 		 to be set at the same time.
		/// 		 Default is a 2D read-only texture
		/// 		 there are multiple buffers types, all rquire CF_BUFFER to be set
		/// 		 Multisample only applies to 2D textures
		/// 		 
		////////////////////////////////////////////////////////////////////////////////////////////////////
		enum CREATION_FLAGS {
			CF_RENDER_TARGET		= BIT(0),	//!< extra view is a render target
			CF_FROM_D3D_TEXTURE		= BIT(1),	//!< d3d texture ptr to wrap 
			CF_MULTISAMPLE			= BIT(2),	//!< has extra samples as an MSAA texture
			CF_UAV					= BIT(3),	//!< extra view is a UAV buffer or texture
			CF_KEYEDMUTEX			= BIT(4),	//!< Use the shared device keyed mutex feature
			CF_PRE_FILL				= BIT(5),	//!< create a pre filled immutable texture
			CF_CPU_READ				= BIT(6),	//!< cpu wants to map and read from the texture
			CF_CPU_WRITE			= BIT(7),	//<! cpu wants to map and write from this texture
			CF_1D					= BIT(8),	//<! texture is 1D only
			CF_2D					= 0,		//!< the default but here for orthogonality
			CF_3D					= BIT(9),	//<! texture is 3D
			CF_CUBE_MAP				= BIT(10),	//<! texture is a cube map
			CF_ARRAY				= BIT(11),	//!< texture has arrays of slices
			CF_BUFFER				= BIT(12),  //<! extra view is a texture buffer type
			CF_RAW_BUFFER			= BIT(13),	//<! buffer is RAW
			CF_STRUCTURED_BUFFER	= BIT(14),	//<! buffer is structured
			CF_UAV_COUNTER			= BIT(15),	//<! Has a counter
			CF_UAV_APPEND			= BIT(16),	//<! appendable/consumable
			CF_DRAW_INDIRECT		= BIT(17),	//<! Can be used for draw/dispatch indirect
			CF_STREAM_OUT			= BIT(18),	//<! Can be a target for stream out
			CF_VERTEX_BUFFER		= BIT(19),	//<! Can be bound as a vertex buffer
		};

	private:

	};


	typedef const Core::AsyncResourceHandle<TextureType, Texture, Texture::InternalAsyncType > TextureHandle;
	typedef TextureHandle* TextureHandlePtr;
	typedef Core::shared_ptr<Texture> TexturePtr;

} // end namespace Graphics

#endif // WIERD_GRAPHICS_TEXTURE_H
