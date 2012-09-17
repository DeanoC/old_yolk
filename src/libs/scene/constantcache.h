#pragma once
//!-----------------------------------------------------
//!
//! \file constantcache.h
//! cache for a bunch of matrices and other consants
//!
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_CONSTANTCACHE_H_ )
#define YOLK_SCENE_CONSTANTCACHE_H_

#include "core/vector_math.h"
#include "scene/databuffer.h"
#include "scene/program.h"
#include "scene/camera.h"

namespace Scene {

	enum CONSTANT_FREQ_BLOCKS {
		CF_STATIC = 0,
		CF_PER_FRAME,
		CF_PER_PIPELINE,
		CF_PER_VIEWS,
		CF_PER_TARGETS,
		CF_STD_OBJECT,

		CF_PER_MATERIAL, // TODO concanate materials via instancing hw

		CF_NUM_BLOCKS,
		CF_USER_BLOCK0 = CF_NUM_BLOCKS,
		CF_USER_BLOCK1,
		CF_USER_BLOCK2,
		CF_USER_BLOCK3,
		CF_USER_BLOCK4,
		CF_USER_BLOCK5,
		CF_USER_BLOCK6,
		CF_USER_BLOCK7,
		CF_USER_BLOCK8,

		CF_MAX_BLOCKS = 16,
	};
	
	enum CONSTANT_VAR_NAME {
		CVN_VIEW = 0,
		CVN_VIEW_INVERSE,
		CVN_VIEW_IT,
		CVN_PROJ,
		CVN_PROJ_INVERSE,
		CVN_PROJ_IT,
		CVN_VIEW_PROJ,
		CVN_VIEW_PROJ_INVERSE,
		CVN_VIEW_PROJ_IT,
		CVN_WORLD,
		CVN_WORLD_INVERSE,
		CVN_WORLD_IT,
		CVN_WORLD_VIEW,
		CVN_WORLD_VIEW_INVERSE,
		CVN_WORLD_VIEW_IT,
		CVN_WORLD_VIEW_PROJ,
		CVN_WORLD_VIEW_PROJ_INVERSE,
		CVN_WORLD_VIEW_PROJ_IT,

		CVN_PREV_WORLD_VIEW_PROJ,	// special case used in reprojection shaders, its the WVP from the previous frame
		CVN_USER_MATRIX_0,			// these user matrixs have no specified use, but shader can interpret them as sees fit
		CVN_USER_MATRIX_1,			// examples include texture transforms, colour shifts etc.

		CVN_NUM_MATRICES,

		CVN_FRAMECOUNT,			// unsigned int vec4
		CVN_TARGET_DIMS,		// unsigned int vec4
		CVN_DUMMY,				// float vec4 (for testing)
		CVN_ZPLANES,			// float4 x=near y=far, far - near
		CVN_FOV,				// float4 fov x and y scale, 

		CVN_MATERIAL_INDEX,
		CVN_LIGHT_COUNTS,		// uint4 numDirectionalLights, 

		CVN_NUM_CONSTANTS
	};


	class ConstantCache {
	public:
		typedef uint64_t				CachedBitFlags;
	
		ConstantCache();
		~ConstantCache();

		void getVector( CONSTANT_VAR_NAME type, uint32_t* out4 ) const;
		void getVector( CONSTANT_VAR_NAME type, float* out4 ) const;
		void setVector( CONSTANT_VAR_NAME type, const uint32_t* in4 );
		void setVector( CONSTANT_VAR_NAME type, const float* in4 );

		const Math::Matrix4x4& getMatrix( CONSTANT_VAR_NAME type ) const;
		const Math::Vector4& getVector( CONSTANT_VAR_NAME type ) const;
		void setVector( CONSTANT_VAR_NAME type, const Math::Vector4& in );

		// for specific times, you may wish to change a specific matrix, without anything else changing
		// this allows it. be afraid, very afraid
		void setMatrixBypassCache( CONSTANT_VAR_NAME type, const Math::Matrix4x4& mat );
		// mark the cache for type invalid 
		void invalidCacheOfType( CONSTANT_VAR_NAME type );

		// change object passed int previous WVP and new world matrix
		void setObject(	const Math::Matrix4x4& prevWVPMatrix = Math::IdentityMatrix(),
							const Math::Matrix4x4& worldMatrix  = Math::IdentityMatrix() );

		// in general the set object and set camera, do the job of setting world/view/projection
		// but in some cases its handy to do directly
		void setWorldMatrix( const Math::Matrix4x4& worldMatrix  = Math::IdentityMatrix() );
		void setViewMatrix( const Math::Matrix4x4& viewMatrix  = Math::IdentityMatrix() );
		void setProjectionMatrix( const Math::Matrix4x4& projMatrix  = Math::IdentityMatrix() );
		
		// set either of the user matrixes
		void setUserMatrix0( const Math::Matrix4x4& userMatrix );
		void setUserMatrix1( const Math::Matrix4x4& userMatrix );

		// set the camera currently used by the renderer
		void setCamera( const Scene::CameraPtr& _camera );

		// updates GPU shared uniform blocks to ensure the needs of the given program
		// callers responsability for user (non shared) blocks, passing null (default)
		// will update all the gpu blocks that need updating, regardless of whether its
		// accessed by the current shader
		void updateGPU( Scene::RenderContext* context, const ProgramPtr prg = nullptr  );
		void updateGPUBlock( Scene::RenderContext* context, CONSTANT_FREQ_BLOCKS block ) const;

		// ensure block is upto date first!
		const DataBufferHandlePtr getBlock( CONSTANT_FREQ_BLOCKS block ) const;

	private:
		void getRawVector( CONSTANT_VAR_NAME type, uint32_t* out4 ) const;
		mutable CachedBitFlags			cachedFlags;
		mutable uint32_t				gpuHasBlocks;
		mutable Math::Matrix4x4			matrixCache[ CVN_NUM_MATRICES ];
		mutable uint8_t					vectorCache[ (CVN_NUM_CONSTANTS - CVN_NUM_MATRICES) * 16]; // 16 bytes per vector
		boost::scoped_array<size_t>		offsets;

		boost::scoped_array<DataBufferHandlePtr>	blockHandles;
	};

}
#endif // YOLK_SCENE_CONSTANTCACHE_H_
