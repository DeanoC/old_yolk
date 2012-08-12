#	pragma once
//!-----------------------------------------------------
//!
//! \file rendermatrixcache.h
//! cache for a bunch of matrices
//!
//!-----------------------------------------------------
#if !defined(WIERD_GRAPHICS_RENDERMATRIXCACHE_H)
#define WIERD_GRAPHICS_RENDERMATRIXCACHE_H

#include "rendercamera.h"

namespace Graphics
{
	// forward decl
	class RenderCamera;

	class RenderMatrixCache {
	public:
		enum MATRIX_TYPE {
			VIEW = 0,
			VIEW_INVERSE,
			VIEW_INVERSE_TRANSPOSE,
			PROJECTION,
			PROJECTION_INVERSE,
			PROJECTION_INVERSE_TRANSPOSE,
			VIEW_PROJECTION,
			VIEW_PROJECTION_INVERSE,
			VIEW_PROJECTION_INVERSE_TRANSPOSE,
			WORLD,
			WORLD_INVERSE,
			WORLD_INVERSE_TRANSPOSE,
			WORLD_VIEW,
			WORLD_VIEW_INVERSE,
			WORLD_VIEW_INVERSE_TRANSPOSE,
			WORLD_VIEW_PROJECTION,
			WORLD_VIEW_PROJECTION_INVERSE,
			WORLD_VIEW_PROJECTION_INVERSE_TRANSPOSE,

			PREVIOUS_WORLD_VIEW_PROJECTION, // special case used in reprojection shaders, its the WVP from the previous frame

			NUM_MATRICES
		};

		static const int NUM_MATRIX_BITS = (1 << NUM_MATRICES);

		RenderMatrixCache();
		const Math::Matrix4x4& getMatrix( MATRIX_TYPE type );

		//! used by meshes to set the current local to world matrix
		void setWorldMatrix( const Math::Matrix4x4& worldMatrix );

		//! used by meshes to set the previous world to projection matrix (i.e. last frames)
		void setPreviousWorldViewProjectionMatrix( const Math::Matrix4x4& wvpMatrix );

		//! we keep an array of matrices to simplify the management of bones, 
		//! these aren't cached and are the callers responsible 
		//! the world matrix is set into the 0th index simplify non-skin'ed
		static const int MAX_BONE_MATRICES = 26;
		Math::Matrix4x4* getBoneMatrixArray() {
			return m_BoneMatrixArray;
		}

		void setCamera( const RenderCameraPtr& camera );
		const RenderCameraPtr& getCamera() const;
	private:
		uint32_t						m_CachedFlags;
		Core::shared_ptr<RenderCamera> m_pCurrentCamera;
		Math::Matrix4x4 m_MatrixCache[ NUM_MATRICES ];
		Math::Matrix4x4 m_BoneMatrixArray[ MAX_BONE_MATRICES ];
	};

}
#endif //WIERD_GRAPHICS_RENDERMATRIXCACHE_H
