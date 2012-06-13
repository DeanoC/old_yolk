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

	//! install a callback to receive Quit message
	void registerQuitCallback( VoidCallback pCallback ) {
		m_pQuitCallback = pCallback;
	}
	//! install a callback to receive DebugMode message
	void registerDebugModeChangeCallback( IntCallback pCallback ) {
		m_pDebugModeChangeCallback = pCallback;
	}

	//! Call a registered callback for Quit
	void quit( void ) {
		if(m_pQuitCallback) {
			m_pQuitCallback();
		}
	}
	//! Call a registered callback for debug mode change
	void debugModeChange( int mode ) {
		if(m_pDebugModeChangeCallback) {
			m_pDebugModeChangeCallback( mode );
		}
	}

public:
	SystemMessage() :
		m_pQuitCallback(0),
		m_pDebugModeChangeCallback(0) {
	}
	~SystemMessage(){}

public:
	VoidCallback m_pQuitCallback;
	IntCallback m_pDebugModeChangeCallback;

};


}	//namespace Core


#endif
