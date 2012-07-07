#pragma once
//!-----------------------------------------------------
//!
//! \file constantcache.h
//! cache for a bunch of matrices and other consants
//!
//!-----------------------------------------------------
#if !defined(WIERD_GL_CONSTANTCACHE_H)
#define WIERD_GL_CONSTANTCACHE_H

#include "core/vector_math.h"
#include "databuffer.h"
#include "program.h"
#include "cl/buffer.h"
#include "scene/camera.h"

namespace Gl {
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

		CVN_PREV_WORLD_VIEW_PROJ, // special case used in reprojection shaders, its the WVP from the previous frame

		CVN_NUM_MATRICES,

		CVN_FRAMECOUNT,			// unsigned int vec4
		CVN_TARGET_DIMS,		// unsigned int vec4
		CVN_DUMMY,				// float vec4 (for testing)
		CVN_ZPLANES,			// float4 x=near y=far, far - near
		CVN_FOV,				// float4 fov x and y scale, 
		CVN_NUM_CONSTANTS
	};


	class ConstantCache {
	public:
		typedef uint64_t				CachedBitFlags;
	
		ConstantCache();
		~ConstantCache();

		const Math::Matrix4x4& getMatrix( CONSTANT_VAR_NAME type ) const;
		const Math::Vector4& getVector( CONSTANT_VAR_NAME type ) const;

		void getUIVector( CONSTANT_VAR_NAME type, uint32_t* out4 ) const;

		void setUIVector( CONSTANT_VAR_NAME type, const uint32_t* in4 );
		void setVector( CONSTANT_VAR_NAME type, const Math::Vector4& in );

		// for specific times, you may wish to change a specific matrix, without anything else changing
		// this allows it. be afraid, very afraid
		void setMatrixBypassCache( CONSTANT_VAR_NAME type, const Math::Matrix4x4& mat );
		// mark the cache for type invalid 
		void invalidCacheOfType( CONSTANT_VAR_NAME type );

		// change object passed int previous WVP and new world matrix
		void changeObject(	const Math::Matrix4x4& prevWVPMatrix = Math::IdentityMatrix(),
							const Math::Matrix4x4& worldMatrix  = Math::IdentityMatrix() );

		void setCamera( const Scene::CameraPtr& _camera ) { camera = _camera; };
		Scene::CameraPtr getCamera() { return camera; }

		// updates GPU shared uniform blocks to ensure the needs of the given program
		// callers responsability for user (non shared) blocks, passing null (default)
		// will update all the gpu blocks that need updating, regardless of whether its
		// accessed by the current shader
		void updateGPU( const ProgramPtr prg = nullptr);
		// fast path if you suspect only object varaibles have changed
		void updateGPUObjectOnly();

		void bind();
		void unbind();

		const Cl::BufferHandlePtr getClBlock( CONSTANT_FREQ_BLOCKS block ) const;

		void updateGPUBlock( CONSTANT_FREQ_BLOCKS block ) const;
	private:
		mutable CachedBitFlags			cachedFlags;
		mutable uint32_t				gpuHasBlocks;
		mutable Math::Matrix4x4			matrixCache[ CVN_NUM_MATRICES ];
		mutable uint8_t					vectorCache[ (CVN_NUM_CONSTANTS - CVN_NUM_MATRICES) * 16]; // 16 bytes per vector
		boost::scoped_array<size_t>		offsets;

		boost::scoped_array<DataBufferHandlePtr>	blockHandles;
		boost::scoped_array<Cl::BufferHandlePtr>	clBlockHandles;

		Scene::CameraPtr				camera;
		static int 						s_cacheCount;
	};

}
#endif // WIERD_GL_CONSTANTCACHE_H
