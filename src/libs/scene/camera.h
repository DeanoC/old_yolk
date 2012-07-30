//!-----------------------------------------------------
//!
//! \file camera.h
//! a camera as seen by the render path
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_CAMERA_H_)
#define YOLK_SCENE_CAMERA_H_

#pragma once

#if !defined(WIERD_CORE_FRUSTUM_H)
#include "core/frustum.h"
#endif

#include "core/simplecachable.h"

namespace Scene {
	class Camera : public Core::SimpleCachable<uint32_t,false>{
	public:
		Camera();

		//! sets the projection of the camera, fov is total not half
		void setProjection( float fov, float aspect, float znear, float zfar );

		void setFOV( float fov, float aspect = 1.0f );
		void setDepthRange( float znear, float zfar );

		//! sets the view of the matrix
		void setView( const Math::Matrix4x4& viewMatrix );

		const Core::Frustum& getFrustum() const;

		const Math::Matrix4x4& getProjection() const { return projectionMatrix; }
		const Math::Matrix4x4& getView() const { return viewMatrix; }
		const float getZNear() const { return zNear; };
		const float getZFar() const { return zFar; };
		const float getXScale() const { return xScale; };
		const float getYScale() const { return yScale; };
		const Math::Vector3 getPosition() const;

	protected:
		void setProjection(); // uses already stored values

		Math::Matrix4x4 viewMatrix;
		Math::Matrix4x4 projectionMatrix;
		float			zNear;
		float			zFar;
		float			xScale;		// the fov xscale
		float			yScale;		// the fov yscale
		mutable Core::SimpleCachable< uint32_t, false> frustumCache;
		mutable Core::Frustum	frustum;
	};

	typedef std::shared_ptr<Camera> CameraPtr;
};

#endif // YOLK_SCENE_CAMERA_H
