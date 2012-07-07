//!-----------------------------------------------------
//!
//! \file camera.cpp
//! a camera class for the renderer
//!
//!-----------------------------------------------------

#include "scene.h"
#include "camera.h"

namespace Scene {

Camera::Camera(){
}

void Camera::setProjection( float fov, float aspect, float znear, float zfar ) {
	yScale = 1.f / tanf(fov/2);
	xScale = aspect / yScale;
	projectionMatrix = Math::Matrix4x4(	xScale,		0,		0,								0,
										0,			yScale,	0,								0,
										0,			0,		(zfar+znear)/(zfar-znear),		1,
										0,			0,		-(2*znear*zfar)/(zfar-znear),	0 );
	zNear = znear;
	zFar = zfar;

	invalidate();
}

void Camera::setView( const Math::Matrix4x4& _viewMatrix ) {
	viewMatrix = _viewMatrix;
	invalidate();
}

const Math::Vector3 Camera::getPosition() const {
	return Math::GetTranslation( Math::InverseMatrix(viewMatrix) );
}

const Core::Frustum& Camera::getFrustum() const { 
	if( frustumCache.getCounter() != getCounter() ) {
		frustum = Core::Frustum( viewMatrix * projectionMatrix );
		frustumCache.setCounter( getCounter() );
	}

	return frustum; 
}

}

