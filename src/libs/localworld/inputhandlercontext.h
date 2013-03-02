#pragma once

#if !defined(YOLK_LOCALWORLD_INPUT_HANDLER_CONTEXT_H)
#define YOLK_LOCALWORLD_INPUT_HANDLER_CONTEXT_H

#include <core/development_context.h>
#include "core/boundedbuffer.h"
#include "inputframe.h"

class SceneWorld;

class InputHandlerContext : public Core::DevelopmentContextInterface {
public:
	InputHandlerContext();

	/// called every frame that the context is active for x movement
	virtual void padXAxisMovement( unsigned int padNum, float x ) { curFrame.pad[padNum].XAxisMovement += x; }
	/// called every frame that the context is active for y movement
	virtual void padYAxisMovement( unsigned int padNum, float y ) { curFrame.pad[padNum].YAxisMovement += y; }
	/// called every frame that the context is active when Button 1 is down.
	virtual void padButton1( unsigned int padNum ) { curFrame.pad[padNum].button1 = true; }
	/// called every frame that the context is active when Button 2 is down.
	virtual void padButton2( unsigned int padNum ) { curFrame.pad[padNum].button2 = true; }
	/// called every frame that the context is active when Button 3 is down.
	virtual void padButton3( unsigned int padNum ) { curFrame.pad[padNum].button3 = true; }
	/// called every frame that the context is active when Button 4 is down.
	virtual void padButton4( unsigned int padNum ) { curFrame.pad[padNum].button4 = true; }

	/// movement along the mouse X axis.
	virtual void mouseX( float x ) { curFrame.mouseX += x; }
	/// movement along the mouse Y axis.
	virtual void mouseY( float y ) { curFrame.mouseY += y; }
	/// Left Mouse Button is down.
	virtual void mouseLeftButton() { curFrame.mouseLeftButton = true; }
	/// Right Mouse Button is down.
	virtual void mouseRightButton() { curFrame.mouseRightButton = true; }

	virtual void absoluteMouseX( float x ) { curFrame.absoluteMouseX = x; }
	virtual void absoluteMouseY( float y ) { curFrame.absoluteMouseY = y; }

	//! call when switched to and from this context.
	virtual void enable( bool on );
	//! called every frame this context is active
	virtual void update( float fTimeInSecs );

	virtual void debugButton1( unsigned int padNum ) { curFrame.pad[padNum].debugButton1 = true; }
	virtual void debugButton2( unsigned int padNum ) { curFrame.pad[padNum].debugButton2 = true; }
	virtual void debugButton3( unsigned int padNum ) { curFrame.pad[padNum].debugButton3 = true; }
	virtual void debugButton4( unsigned int padNum ) { curFrame.pad[padNum].debugButton4 = true; }
	virtual void debugButton5( unsigned int padNum ) { curFrame.pad[padNum].debugButton5 = true; }
		
	// TODO per screen/view lookup for now only one camera for N controllers
	virtual std::shared_ptr<Scene::Camera> getCamera() const { return camera; }

	void setCamera( std::shared_ptr<Scene::Camera> _camera ) { camera = _camera; }
	bool dequeueInputFrame( InputFrame* frame ); 	//!< return false if no input to dequeue

protected:
	void queueInputFrame( const InputFrame& frame );
	std::shared_ptr<Scene::Camera>	camera;
	SceneWorld* 					owner;
	InputFrame						curFrame;
	Core::BoundedBuffer<InputFrame>				inputQueue;
};


#endif // end YOLK_DWM_CLIENT_INPUT_HANDLER_CONTEXT_H