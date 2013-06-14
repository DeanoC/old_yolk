#pragma once
#if !defined(HOUSE_DEBUG_CAM_CONTEXT_H)
#define HOUSE_DEBUG_CAM_CONTEXT_H

#include <core/development_context.h>


namespace Scene {
	class Camera;
}

class DebugCamContext : public Core::DevelopmentContextInterface {
public:
	DebugCamContext( int scrWidth, int scrHeight, float degFov, float znear, float zfar );
	~DebugCamContext();

	/// called every frame that the context is active for x movement
	virtual void padXAxisMovement( unsigned int padNum, float x ) override;
	/// called every frame that the context is active for y movement
	virtual void padYAxisMovement( unsigned int padNum, float y ) override;
	/// called every frame that the context is active when Button 1 is down.
	virtual void padButton1( unsigned int padNum ) override;
	/// called every frame that the context is active when Button 2 is down.
	virtual void padButton2( unsigned int padNum ) override;
	/// called every frame that the context is active when Button 3 is down.
	virtual void padButton3( unsigned int padNum ) override;
	/// called every frame that the context is active when Button 4 is down.
	virtual void padButton4( unsigned int padNum ) override;

	/// movement along the mouse X axis.
	virtual void mouseX( float x ) override;
	/// movement along the mouse Y axis.
	virtual void mouseY( float y ) override;
	/// Left Mouse Button is down.
	virtual void mouseLeftButton() override;
	/// Right Mouse Button is down.
	virtual void mouseRightButton() override;

	//! call when switched to and from this context.
	virtual void enable( bool on ) override;
	//! called every frame this context is active
	virtual void update( float fTimeInSecs ) override;

	//! Context will call for your to display you debug info.
	virtual void display() override;

	virtual std::shared_ptr<Scene::Camera> getCamera() const override { return camera; }

	virtual void debugButton1( unsigned int padNum ) override;
	virtual void debugButton2( unsigned int padNum ) override;
	virtual void debugButton3( unsigned int padNum ) override;
	virtual void debugButton4( unsigned int padNum ) override;
	virtual void debugButton5( unsigned int padNum ) override;

	int		debugLevel;
	void 	setAspectRatio( int scrWidth, int scrHeight, int winWidth, int winHeight );

protected:
	//! Render camera 
	std::shared_ptr<Scene::Camera> camera;
	float fovRads; 

	float dxRot;
	float dyRot;
	float dzRot;

	float xRot;
	float yRot;
	float zRot;
	float speed;
	float angularSpeed;
	Math::Vector3 position;

	float curForwardMotion;
	float curSideMotion;

	bool	lockedFrustum;
};


#endif // end HOUSE_DEBUG_CAM_CONTEXT_H