//!-----------------------------------------------------
//!
//! \file camera.cpp
//! a camera class for the renderer
//!
//!-----------------------------------------------------

#include "scene.h"
#include "camera.h"

namespace Scene {

Camera::Camera() : 
	yScale( 1.0f ),
	xScale( 1.0f ),
	zNear( 0.1 ),
	zFar( 4000 ) {
}

void Camera::setProjection( float fov, float aspect, float znear, float zfar ) {
	yScale = 1.f / tanf(fov/2);
	xScale = aspect / yScale;
	zNear = znear;
	zFar = zfar;
	setProjection();
}

void Camera::setFOV( float fov, float aspect ) {
	yScale = 1.f / tanf(fov/2);
	xScale = aspect / yScale;

	setProjection();
}

void Camera::setDepthRange( float _near, float _far ) {
	zNear = _near;
	zFar = _far;

	setProjection();
}

void Camera::setProjection() {
	projectionMatrix = Math::Matrix4x4(	xScale,		0,		0,								0,
										0,			yScale,	0,								0,
										0,			0,		(zFar+zNear)/(zFar-zNear),		1,
										0,			0,		-(2*zNear*zFar)/(zFar-zNear),	0 );
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

