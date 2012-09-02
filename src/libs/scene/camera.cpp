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
	ortho( false ),
	yScale( 1.0f ),
	xScale( 1.0f ),
	zNear( 0.1f ),
	zFar( 4000.0f ) {
}

void Camera::setProjection( float fov, float aspect, float znear, float zfar ) {
	yScale = 1.f / tanf(fov/2);
	xScale = aspect / yScale;
	zNear = znear;
	zFar = zfar;
	ortho = false;
	setProjection();
}
void Camera::setOrthographic( float _left, float _right, 
							float _top, float _bottom, 
							float _znear, float _zfar ) {
	left = _left;
	right = _right;
	top = _top;
	bottom = _bottom;
	zNear = _znear;
	zFar = _zfar;
	ortho = true;
	setOrthographic();
}

void Camera::setFOV( float fov, float aspect ) {
	yScale = 1.f / tanf(fov/2);
	xScale = aspect / yScale;

	ortho = false;
	setProjection();
}

void Camera::setDepthRange( float _near, float _far ) {
	zNear = _near;
	zFar = _far;

	if( ortho ) {
		setOrthographic();
	} else {
		setProjection();
	}
}

void Camera::setProjection() {
	projectionMatrix = Math::Matrix4x4(	
		xScale,		0,		0,								0,
		0,			yScale,	0,								0,
		0,			0,		(zFar+zNear)/(zFar-zNear),		1,
		0,			0,		-(2*zNear*zFar)/(zFar-zNear),	0 
	);
	invalidate();		
}

void Camera::setOrthographic() {
	projectionMatrix = Math::Matrix4x4( 
		2.0f / (right - left), 	0, 						0, 							0, 
		0, 						2.0f / (top - bottom), 	0, 							0,
		0, 						0, 						-2.0f / (zFar - zNear), 	0,
		-(right+left)/(right-left), -(top+bottom)/(top-bottom), -(zFar+zNear)/(zFar-zNear), 1 
	);
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
	//if( frustumCache.getCounter() != getCounter() ) 
	{
		frustum = Core::Frustum( viewMatrix * projectionMatrix );
		frustumCache.setCounter( getCounter() );
	}

	return frustum; 
}

}

