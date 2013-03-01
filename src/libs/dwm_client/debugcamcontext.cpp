//!-----------------------------------------------------
//!
//! \file debugcamcontext.cpp
//!
//!-----------------------------------------------------
#include <core/core.h>
#include <core/debug_render.h>
#include <core/sysmsg.h>
#include "scene/camera.h"
#include "scene/rendercontext.h"
#include "clientworld.h"
#include "debugcamcontext.h"

DebugCamContext::DebugCamContext( int scrWidth, int scrHeight, float degFov, float znear, float zfar ) :
	camera( CORE_NEW Scene::Camera() )
{
	float aspect = (float)scrWidth / (float)scrHeight;
	fovRads = Math::degree_to_radian<float>() * degFov;
	camera->setProjection( fovRads, aspect, znear, zfar );

	// default
	xRot = yRot = zRot = 0;
	curSideMotion = curForwardMotion = 0;
	speed = 50.0f;
	angularSpeed = 90.f; // degrees per second
	position = Math::Vector3(0,0,10);
	camera->setView( Math::IdentityMatrix() );

	lockedFrustum = false;
	debugLevel = 0;
}
DebugCamContext::~DebugCamContext() {	
}

void DebugCamContext::padXAxisMovement( unsigned int padNum, float x ) {
	if( padNum == 0 ) {
		curSideMotion += x * speed;
	}
}

void DebugCamContext::padYAxisMovement( unsigned int padNum, float y ) {
	if( padNum == 0 ) {
		curForwardMotion += y * speed;
	}
}

void DebugCamContext::padButton1( unsigned int padNum ) {
}

void DebugCamContext::padButton2( unsigned int padNum ) {
}

void DebugCamContext::padButton3( unsigned int padNum ) {
}

void DebugCamContext::padButton4( unsigned int padNum ) {
}

void DebugCamContext::debugButton1( unsigned int padNum ) {

	debugLevel = (debugLevel+1) % 3;
	Core::SystemMessage::get()->debugModeChange( debugLevel );

	std::stringstream buf;
	buf << "Debug Mode : " << debugLevel;
//	Graphics::ScrConsole::Get()->Print( buf.str() );
}

void DebugCamContext::debugButton2( unsigned int padNum ) {
//	Graphics::ScrConsole::Get()->Print( "Frustum Lock flipped" );
	lockedFrustum ^= true;
}

void DebugCamContext::debugButton3( unsigned int padNum ) {
}

void DebugCamContext::debugButton4( unsigned int padNum ) {
}

void DebugCamContext::debugButton5( unsigned int padNum ) {
}


void DebugCamContext::mouseDeltaX( float x ) {
	yRot += x * angularSpeed;
}

void DebugCamContext::mouseDeltaY( float y ) {
	xRot  += y * angularSpeed;
}

void DebugCamContext::mouseLeftButton() {
}

void DebugCamContext::mouseRightButton() {
}

void DebugCamContext::enable( bool on  ) {
	if( on ) {
		// this camera stuff sucked, needs a rewrite
//		owner->setActiveCamera( camera );
//		controlContext->setCamera( camera );
	}
}

void DebugCamContext::update( float fTimeInSecs ) {
	using namespace Math;

	Matrix4x4 mat,xrot,yrot,zrot;
	xrot = CreateXRotationMatrix( degree_to_radian<float>() * xRot );
	yrot = CreateYRotationMatrix( degree_to_radian<float>() * yRot );
	zrot = CreateZRotationMatrix( degree_to_radian<float>() * zRot );
	mat = MultiplyMatrix( xrot, yrot );
	mat = MultiplyMatrix( mat, zrot );

	Vector3 xvec = Math::GetXAxis( mat );
	Vector3 yvec = Math::GetYAxis( mat );
	Vector3 zvec = Math::GetZAxis( mat );

	// move along view direction
	position -= (xvec * curSideMotion * fTimeInSecs);
	position += (zvec * curForwardMotion * fTimeInSecs );

	// make view matrix
	camera->setView( CreateLookAtMatrix( position, position + zvec, yvec ) );

//	if( lockedFrustum == false ) {
//		m_pCamera->UpdateFrustum();
//	}

	// reset motion
	curSideMotion = curForwardMotion = 0;
}

void DebugCamContext::display() {
	using namespace Core;
	using namespace Math;

	g_pDebugRender->worldLine( RGBAColour(1,0,0,1), Vector3(0,0,0), Vector3(100,0,0) );
	g_pDebugRender->worldLine( RGBAColour(0,1,0,1), Vector3(0,0,0), Vector3(0,100,0) );
	g_pDebugRender->worldLine( RGBAColour(0,0,1,1), Vector3(0,0,0), Vector3(0,0,100) );

//	owner->debugDraw( controlContext );

	//	m_pCamera->getFrustum().debugDraw( RGBAColour(1,1,1,1) );
}

void DebugCamContext::setAspectRatio( int scrWidth, int scrHeight, int winWidth, int winHeight ) {

	float winAspect = (float) winWidth / (float) winHeight;
	float scrAspect = (float) scrWidth / (float) scrHeight;
	float aspect = scrAspect / winAspect;

	camera->setProjection( fovRads, aspect, camera->getZNear(), camera->getZFar() );

}
