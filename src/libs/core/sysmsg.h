//!-----------------------------------------------------
//!
//! \file sysmsg.h
//! Communicates important messages (like quitting etc.)
//! from the engine to the game
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_SYSMSG_H
#define WIERD_CORE_SYSMSG_H


namespace Core
{


class SystemMessage : public Singleton< SystemMessage >
{
public:
	typedef void (*VoidCallback)( );
	typedef void (*BoolCallback)( bool );
	typedef void (*IntCallback)( int );
	typedef void (*Int2Callback)( int, int );

	//! install a callback to receive Quit message
	void registerQuitCallback( VoidCallback pCallback ) {
		quitCallback = pCallback;
	}
	//! install a callback to receive DebugMode message
	void registerDebugModeChangeCallback( IntCallback pCallback ) {
		debugModeChangeCallback = pCallback;
	}
	//! install a callback to receive resize message
	void registerResizeCallback( Int2Callback pCallback ) {
		resizeCallback = pCallback;
	}

	//! Call a registered callback for Quit
	void quit( void ) { if(quitCallback) { quitCallback(); } }
	//! Call a registered callback for debug mode change
	void debugModeChange( int mode ) { if(debugModeChangeCallback) { debugModeChangeCallback( mode ); }	}

	void resize( int width, int height ) { if(resizeCallback) { resizeCallback( width, height ); } }

public:
	SystemMessage() :
		quitCallback(0),
		debugModeChangeCallback(0),
		resizeCallback(0) {
	}

	~SystemMessage(){}

public:
	VoidCallback quitCallback;
	IntCallback debugModeChangeCallback;
	Int2Callback resizeCallback;

};


}	//namespace Core


#endif
