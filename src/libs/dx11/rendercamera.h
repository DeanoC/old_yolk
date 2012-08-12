//!-----------------------------------------------------
//!
//! \file rendercamera.h
//! a camera as seen by the graphics engine
//!
//!-----------------------------------------------------
#if !defined(WIERD_GRAPHICS_RENDERCAMERA_H)
#define WIERD_GRAPHICS_RENDERCAMERA_H

#pragma once

#if !defined(WIERD_CORE_FRUSTUM_H)
#include "core/frustum.h"
#endif

namespace Graphics {
	class RenderCamera {
	public:
		friend class RenderMatrixCache;
		RenderCamera();

		//! sets the projection of the camera, fov is total not half
		void SetProjection( float fov, float aspect, float znear, float zfar );
		void SetView( const Math::Matrix4x4& viewMatrix );
		void UpdateFrustum();

		Core::Frustum* GetFrustum() {
			return &m_Frustum;
		}
		const float getZNear() const { return m_zNear; };
		const float getZFar() const { return m_zFar; };
		const Math::Vector3 GetPosition() const;

	protected:
		bool			m_bCached;
		Math::Matrix4x4 m_ViewMatrix;
		Math::Matrix4x4 m_ProjectionMatrix;
		Core::Frustum	m_Frustum;
		float			m_zNear;
		float			m_zFar;
	};
	typedef Core::shared_ptr<RenderCamera> RenderCameraPtr;
};

#endif // WIERD_GRAPHICS_RENDERCAMERA_H
