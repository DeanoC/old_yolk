#pragma once
#if !defined( YOLK_SCENE_RESOURCE_H_ )
#define YOLK_SCENE_RESOURCE_H_

#include "scene/generictextureformat.h"

namespace Scene {

	///-------------------------------------------------------------------------------------------------
	/// \enum	RESOURCE_CREATION_FLAGS
	///
	/// \brief	Values that represent texture or buffer creation details. 
	///
	/// 		 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	enum RESOURCE_CREATION_FLAGS {
		RCF_BUF_CONSTANT			= BIT(0),	//!< can be bound as a constant buffer.
		RCF_BUF_VERTEX				= BIT(1),	//!< can be bound as vertex data buffer.
		RCF_BUF_INDEX				= BIT(2),	//!< can be bound as index data buffer.
		RCF_BUF_STREAMOUT			= BIT(3),	//!< can be bound as stream out data buffer.
		RCF_BUF_GENERAL				= BIT(4),	//!< as general buffer (for GL)
		RCF_TEX_1D					= BIT(5),	//<! texture is 1D
		RCF_TEX_2D					= BIT(6),	//!< 2D texture
		RCF_TEX_3D					= BIT(7),	//<! texture is 3D
		RCF_TEX_CUBE_MAP			= BIT(8),	//<! texture is a cube map

		RCF_PRG_READ 				= BIT(9),	//!< can be bound to be read by a program
		RCF_PRG_STRUCTURED			= BIT(10),	//!< can have structured data in it (SRV in Dx11 parlence GL shader buffer)
		RCF_PRG_BYTE_ACCESS			= BIT(11),	//!< byte level access to the resource in a program
		RCF_PRG_UA_CONSUME			= BIT(12),	//<! dx11 only uav consumable buffer

		RCF_OUT_UNORDERED_ACCESS	= BIT(13),	//!< can be randomly access in a program (UAV in Dx11, Image in GL)
		RCF_OUT_RENDER_TARGET		= BIT(14),	//!< can be used as a render target
		RCF_OUT_UA_COUNTER			= BIT(15),	//<! dx11 only uav counter
		RCF_OUT_UA_APPEND			= BIT(16),	//<! dx11 only uav appendable buffer

		RCF_ACE_IMMUTABLE			= BIT(17),	//!< create a pre filled immutable buffer
		RCF_ACE_CPU_WRITE			= BIT(18),	//!< CPU updates this buffer default=regularly
		RCF_ACE_CPU_READ			= BIT(19),	//!< Direct CPU reading of this surface (probably very slow or even not allowed)
		RCF_ACE_CPU_STAGING			= BIT(20),	//!< buffer is for CPU to read back GPU data
		RCF_ACE_GPU_WRITE_ONLY		= BIT(21),	//!< GPU updates buffer via copy/renders etc only.
		RCF_ACE_INTERMITANT			= BIT(22),	//!< updates/readbacks are irregular time wise
		RCF_ACE_ONCE				= BIT(23),	//!< update/readbacks happens only once
		RCF_ACE_GPU_INDIRECT		= BIT(24),	//!< can be consumed by the GPU for IndirectDraw* calls

		RCF_D3D_FROM_OS				= BIT(25),	//! D3D specific, wraps an existing D3D11TexturexD* (must specify 2D etc.) 

		RCF_MISC_DELETE_FINISH		= BIT(26),	//! CORE_DELETE teh poitner at deleteWhenFinish when load/create is done
	};

	class Resource {
	public:
		//!< used for both databuffers and textures now! obviously not all field apply to both
		struct CreationInfo {
			CreationInfo() : flags((RESOURCE_CREATION_FLAGS)0), width(0), height(1), depth(1), slices(1), mipLevels(1), samples(1), deleteWhenFinish( nullptr ) {}

			RESOURCE_CREATION_FLAGS	flags;			//!< RESOURCE_CREATION_FLAGS

			uint32_t	width;						//!< create a texture of this width or size in bytes of a databuffer
			uint32_t	height;						//!< create a texture of this height
			uint32_t	depth;						//!< 3D depth
			uint32_t 	slices;						//!< slice count
			uint32_t	mipLevels;					//!< how many mip levels (0=all)
			uint32_t	samples;					//!< number of samples for AA

			GENERIC_TEXTURE_FORMAT	format;			//!< GENERIC_TEXTURE_FORMAT of the wanted texture
			const void*	prefillData;			//!< RCF_ACE_IMMUTABLE this is the data that will be copied into the texture
			uint32_t	prefillPitch;			//!< RCF_ACE_IMMUTABLE this is the pitch from line to line of the 2D prefill data
			const void*	referenceTex;			//!< A few creation modes needs to reference another texture, RCD_D3D_FROM_OS uses D3D11Texture*

			void*		deleteWhenFinish;		//!< CORE_DELETE on this when its loaded, make async memory management a bit easier
			uint32_t	structureSize;			//!< TODO size of a structure for structured buffers
		};

		static CreationInfo BufferCtor( 	uint32_t flags, uint32_t width, const void* prefillData = nullptr );

		static CreationInfo ViewCtor( 	uint32_t flags, uint32_t width, uint32_t height = 0, uint32_t depth = 0, uint32_t slices = 0, 
											uint32_t mipLevels = 0, uint32_t sampleCount = 0, GENERIC_TEXTURE_FORMAT fmt = GTF_UNKNOWN );

	private:
		Resource();
	};

	inline Resource::CreationInfo Resource::BufferCtor( 	uint32_t flags, uint32_t width, const void* prefillData ) {
		CORE_ASSERT( flags & (RCF_BUF_CONSTANT | RCF_BUF_VERTEX | RCF_BUF_INDEX | RCF_BUF_STREAMOUT | RCF_BUF_GENERAL) );
		CreationInfo cs;
		cs.flags = (RESOURCE_CREATION_FLAGS)flags;
		cs.width = width;
		cs.height = 0;
		cs.depth = 0;
		cs.slices = 0;
		cs.mipLevels = 0;
		cs.samples = 0;
		cs.format = GTF_UNKNOWN;
		cs.prefillData = prefillData;
		cs.prefillPitch = 0;
		cs.referenceTex = nullptr;
		return cs;
	}

	inline Resource::CreationInfo Resource::ViewCtor( 	uint32_t flags, uint32_t width, uint32_t height, uint32_t depth, uint32_t slices, 
															uint32_t mipLevels, uint32_t samples, GENERIC_TEXTURE_FORMAT fmt ) {
		CreationInfo cs;
		cs.flags = (RESOURCE_CREATION_FLAGS)flags;
		cs.width = width;
		cs.height = height;
		cs.depth = depth;
		cs.slices = slices;
		cs.mipLevels = mipLevels;
		cs.samples = samples;
		cs.format = fmt;
		cs.prefillData = nullptr;
		cs.prefillPitch = 0;
		cs.referenceTex = nullptr;
		return cs;
	}
}

#endif