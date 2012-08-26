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
		return avgXPos;
	}
	float getYPos() {
		return avgYPos;
	}
	bool getLeftMouseButton() {
		return bLeftButton;
	}
	bool getRightMouseButton() {
		return bRightButton;
	}
protected:
	void averageSamples();

	bool bLeftButton;
	bool bRightButton;
	bool bMiddleButton;
	bool bSideButton1;
	bool bSideButton2;

	int numSamples;
	int nMouseWheelDelta;
	int midX;
	int midY;
	int xPos;
	int yPos;
	float time;
	float lastTime;
	int avgMouseWheelDelta;
	int avgXPos;
	int avgYPos;

};


}	//namespace Core


#endif
