#pragma once
#if !defined(HOUSE_DEBUG_CAM_CONTEXT_H)
#define HOUSE_DEBUG_CAM_CONTEXT_H

#include <core/development_context.h>

#include <scene/camera.h>
#include <scene/rendercontext.h>

class DebugCamContext : public Core::DevelopmentContextInterface {
public:
	DebugCamContext( Scene::RenderContext* _controlContext, int scrWidth, int scrHeight, float degFov, float znear, float zfar );

	/// called every frame that the context is active for x movement
	virtual void padXAxisMovement( unsigned int padNum, float x );
	/// called every frame that the context is active for y movement
	virtual void padYAxisMovement( unsigned int padNum, float y );
	/// called every frame that the context is active when Button 1 is down.
	virtual void padButton1( unsigned int padNum );
	/// called every frame that the context is active when Button 2 is down.
	virtual void padButton2( unsigned int padNum );
	/// called every frame that the context is active when Button 3 is down.
	virtual void padButton3( unsigned int padNum );
	/// called every frame that the context is active when Button 4 is down.
	virtual void padButton4( unsigned int padNum );

	/// movement along the mouse X axis.
	virtual void mouseDeltaX( float x );
	/// movement along the mouse Y axis.
	virtual void mouseDeltaY( float y );
	/// Left Mouse Button is down.
	virtual void mouseLeftButton();
	/// Right Mouse Button is down.
	virtual void mouseRightButton();

	//! call when switched to and from this context.
	virtual void enable( bool on );
	//! called every frame this context is active
	virtual void update( float fTimeInSecs );

	//! Context will call for your to display you debug info.
	virtual void display();

	virtual void debugButton1( unsigned int padNum );
	virtual void debugButton2( unsigned int padNum );
	virtual void debugButton3( unsigned int padNum );
	virtual void debugButton4( unsigned int padNum );
	virtual void debugButton5( unsigned int padNum );

	int		debugLevel;
	void 	setAspectRatio( int scrWidth, int scrHeight, int winWidth, int winHeight );

protected:
	//! Render camera 
	std::shared_ptr<Scene::Camera> pCamera;
	float fovRads; 

	float xRot;
	float yRot;
	float zRot;
	float speed;
	float angularSpeed;
	Math::Vector3 position;

	float curForwardMotion;
	float curSideMotion;

	Scene::RenderContext* controlContext;

	bool	lockedFrustum;

};


#endif // end HOUSE_DEBUG_CAM_CONTEXT_H