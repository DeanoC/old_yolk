//!-----------------------------------------------------
//!
//! \file mouse_win.h
//!
//!-----------------------------------------------------

#pragma once
#ifndef YOLK_CORE_MOUSE_WIN_H_
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

	// on windows mouse can only be relative or absolute at any one time.
	void setRelativeMode() {
		relativeMode = true;
		lockToWindow();		// no choice if you use relative mode on windows
	}

	void setAbsoluteMode() {
		relativeMode = false;
	}

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
	bool relativeMode;

	int xAbs;
	int yAbs;


};


}	//namespace Core


#endif
