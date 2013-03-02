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

	float getRelX() const {
		return (float)xRel;
	}
	float getRelY() const {
		return (float)yRel;
	}

	float getAbsX() const {
		return (float) xAbs;
	}
	float getAbsY() const {
		return (float) yAbs;
	}

	bool getLeftMouseButton() const {
		return bLeftButton;
	}
	bool getRightMouseButton() const {
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
	int xRel;
	int yRel;
	bool hasFocus;
	bool cursorVisible;

	int xAbs;
	int yAbs;


};


}	//namespace Core


#endif
