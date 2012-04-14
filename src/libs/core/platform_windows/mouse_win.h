//!-----------------------------------------------------
//!
//! \file mouse_win.h
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_MOUSE_WIN_H
#define WIERD_CORE_MOUSE_WIN_H 


#if defined( USE_DINPUT )
#	define DIRECTINPUT_VERSION 0x800
#	include <dinput.h>
#endif


namespace Core
{

class MouseWin : public Singleton<MouseWin>
{
#if defined( USE_DINPUT )
protected:
	LPDIRECTINPUT8			m_diDevice;
	LPDIRECTINPUTDEVICE8A	m_diMouse;
	DIMOUSESTATE2			m_mouseData;
#endif

public:
	MouseWin();
	~MouseWin();
	void update();
	void processMouseMessages( UINT uMsg, WPARAM wParam, LPARAM lParam );

	void lockToWindow();
	void unlockFromWindow();
	void showCursor();
	void hideCursor();

#if defined( USE_DINPUT )
	float getDeltaXAxis() {
		return float(m_mouseData.lX)/ 255.f;
	}
	float getDeltaYAxis() {
		return float(m_mouseData.lY)/ 255.f;
	}
	float getDeltaZAxis() {
		return float(m_mouseData.lZ)/ 255.f;
	}
	bool getLeftMouseButton() {
		return (m_mouseData.rgbButtons[0] != 0);
	}
	bool getRightMouseButton() {
		return (m_mouseData.rgbButtons[1] != 0);
	}
#else
	float getDeltaXAxis() {
		return float(xPos - lastXPos) / 255.f;
	}
	float getDeltaYAxis() {
		return float(yPos - lastYPos) / 255.f;
	}
	bool getLeftMouseButton() {
		return bLeftButton;
	}
	bool getRightMouseButton() {
		return bRightButton;
	}
#endif

protected:
#if !defined( USE_DINPUT )
	bool bLeftButton;
	bool bRightButton;
	bool bMiddleButton;
	bool bSideButton1;
	bool bSideButton2;
	int nMouseWheelDelta;
	int xPos;
	int yPos;
	int lastXPos;
	int lastYPos;
#endif

};


}	//namespace Core


#endif
