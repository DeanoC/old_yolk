//!-----------------------------------------------------
//!
//! \file input_listener.h
//! sub-classes of these get access to the input systems
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_INPUT_LISTENER_H
#define WIERD_CORE_INPUT_LISTENER_H


namespace Core
{


//! An input listener is a package for all inputs
//! override any your interested in
class InputListener
{
public:
	virtual ~InputListener() {}
	/// movement along the X axis.
	virtual void padXAxisMovement( unsigned int padNum, float x ) {}
	/// movement along the Y axis.
	virtual void padYAxisMovement( unsigned int padNum, float y ) {}
	/// Button 1 is down.
	virtual void padButton1( unsigned int padNum ) {}
	/// Button 2 is down.
	virtual void padButton2( unsigned int padNum ) {}
	/// Button 3 is down.
	virtual void padButton3( unsigned int padNum ) {}
	/// Button 4 is down.
	virtual void padButton4( unsigned int padNum ) {}

	virtual void debugButton1( unsigned int padNum ) {}
	virtual void debugButton2( unsigned int padNum ) {}
	virtual void debugButton3( unsigned int padNum ) {}
	virtual void debugButton4( unsigned int padNum ) {}
	virtual void debugButton5( unsigned int padNum ) {}

	/// movement along the mouse X axis.
	virtual void mouseX( float x ) {}
	/// movement along the mouse Y axis.
	virtual void mouseY( float y ) {}
	/// Left Mouse Button is down.
	virtual void mouseLeftButton() {}
	/// Right Mouse Button is down.
	virtual void mouseRightButton() {}

};


}	//namespace Core


#endif
