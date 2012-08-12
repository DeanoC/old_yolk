//!-----------------------------------------------------
//!
//! \file rendercamera.cpp
//! a camera class for the renderer
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "graphics.h"
#include "rendercamera.h"

namespace Graphics
{

RenderCamera::RenderCamera() :
	m_bCached( false )
{
}

void RenderCamera::SetProjection( float fov, float aspect, float znear, float zfar )
{
	float yScale = 1.f / tanf(fov/2);
	float xScale = aspect / yScale;
	m_ProjectionMatrix = Math::Matrix4x4(	xScale,		0,		0,							0,
											0,			yScale,	0,							0,
											0,			0,		zfar/(zfar-znear),			1,
											0,			0,		-znear*zfar/(zfar-znear),	0 );
	m_zNear = znear;
	m_zFar = zfar;
	m_bCached = false;
}

void RenderCamera::SetView( const Math::Matrix4x4& viewMatrix )
{
	m_ViewMatrix = viewMatrix;
	m_bCached = false;
}

const Math::Vector3 RenderCamera::GetPosition() const {
	return Math::GetTranslation( Math::InverseMatrix(m_ViewMatrix) );
}

void RenderCamera::UpdateFrustum() {
	m_Frustum = Core::Frustum( m_ViewMatrix * m_ProjectionMatrix );
}


}

