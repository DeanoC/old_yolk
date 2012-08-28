//!-----------------------------------------------------
//!
//! \file mouse_win.h
//!
//!-----------------------------------------------------

#pragma once
#ifndef YOLK_ORE_MOUSE_WIN_H_
#define YOLK_CORE_MOUSE_WIN_H_

namespace Core {

class MouseWin : public Singleton<MouseWin> {
public:
	MouseWin();
	~MouseWin();
	void update();
	void processMouseMessages( UINT uMsg, WPARAM wParam, LPARAM lParam );

	void lockToWindow();
	void unlockFromWindow();
	void showCursor();
	void hideCursor();

	float getXPos() {
		return (float)xPos;
	}
	float getYPos() {
		return (float)yPos;
	}
	bool getLeftMouseButton() {
		return bLeftButton;
	}
	bool getRightMouseButton() {
		return bRightButton;
	}
protected:
	bool bLeftButton;
	bool bRightButton;
	bool bMiddleButton;
	bool bSideButton1;
	bool bSideButton2;

	int nMouseWheelDelta;
	int midX;
	int midY;
	int xPos;
	int yPos;
	bool hasFocus;
	bool cursorVisible;

};


}	//namespace Core


#endif
