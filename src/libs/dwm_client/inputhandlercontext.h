#pragma once

#if !defined(YOLK_DWM_CLIENT_INPUT_HANDLER_CONTEXT_H)
#define YOLK_DWM_CLIENT_INPUT_HANDLER_CONTEXT_H

#include <core/development_context.h>
#include "dwm_client/inputframe.h"

class ClientWorld;

class InputHandlerContext : public Core::DevelopmentContextInterface {
public:
	InputHandlerContext( ClientWorld* _owner );

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
	virtual void mouseDeltaX( float x ) { curFrame.mouseDeltaX += x; }
	/// movement along the mouse Y axis.
	virtual void mouseDeltaY( float y ) { curFrame.mouseDeltaY += y; }
	/// Left Mouse Button is down.
	virtual void mouseLeftButton() { curFrame.mouseLeftButton = true; }
	/// Right Mouse Button is down.
	virtual void mouseRightButton() { curFrame.mouseRightButton = true; }

	//! call when switched to and from this context.
	virtual void enable( bool on );
	//! called every frame this context is active
	virtual void update( float fTimeInSecs );

	virtual void debugButton1( unsigned int padNum ) { curFrame.pad[padNum].debugButton1 = true; }
	virtual void debugButton2( unsigned int padNum ) { curFrame.pad[padNum].debugButton2 = true; }
	virtual void debugButton3( unsigned int padNum ) { curFrame.pad[padNum].debugButton3 = true; }
	virtual void debugButton4( unsigned int padNum ) { curFrame.pad[padNum].debugButton4 = true; }
	virtual void debugButton5( unsigned int padNum ) { curFrame.pad[padNum].debugButton5 = true; }

protected:
	ClientWorld* 					owner;
	InputFrame						curFrame;
};


#endif // end YOLK_DWM_CLIENT_INPUT_HANDLER_CONTEXT_H