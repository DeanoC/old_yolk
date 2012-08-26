//!-----------------------------------------------------
//!
//! \file mouse_win.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "core/clock.h"
#include "mouse_win.h"
extern HWND g_hWnd;

namespace Core {

/**
MouseWin
*/
MouseWin::MouseWin() : numSamples(0) {
	update();
	lastTime = Clock::time( Clock::get()->getInstantTicks() );

	lockToWindow();
	hideCursor();

	POINT pt;
	GetPhysicalCursorPos( &pt );
	midX = pt.x;
	midY = pt.y;
	xPos = midX;
	yPos = midY;		
	
}

MouseWin::~MouseWin() {
}

void MouseWin::update() {
	averageSamples();
}

void MouseWin::processMouseMessages( UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	bLeftButton = false;
	bRightButton = false;
	bMiddleButton = false;
	bSideButton1 = false;
	bSideButton2 = false;
	// Consolidate the mouse button messages and pass them to the app's mouse callback
	if( uMsg == WM_LBUTTONDOWN ||
		uMsg == WM_LBUTTONUP ||
		uMsg == WM_LBUTTONDBLCLK ||
		uMsg == WM_MBUTTONDOWN ||
		uMsg == WM_MBUTTONUP ||
		uMsg == WM_MBUTTONDBLCLK ||
		uMsg == WM_RBUTTONDOWN ||
		uMsg == WM_RBUTTONUP ||
		uMsg == WM_RBUTTONDBLCLK ||
		uMsg == WM_XBUTTONDOWN ||
		uMsg == WM_XBUTTONUP ||
		uMsg == WM_XBUTTONDBLCLK ||
		uMsg == WM_MOUSEWHEEL ||
		uMsg == WM_MOUSEMOVE ) {


		if( uMsg == WM_MOUSEMOVE ) {
		} else {	
			// WM_MOUSEWHEEL passes screen mouse coords
			nMouseWheelDelta += ( short )HIWORD( wParam );
		}
	
		// positive vote (no button looses)
		int nMouseButtonState = LOWORD( wParam );
		bLeftButton |= ( ( nMouseButtonState & MK_LBUTTON ) != 0 );
		bRightButton |= ( ( nMouseButtonState & MK_RBUTTON ) != 0 );
		bMiddleButton |= ( ( nMouseButtonState & MK_MBUTTON ) != 0 );
		bSideButton1 |= ( ( nMouseButtonState & MK_XBUTTON1 ) != 0 );
		bSideButton2 |= ( ( nMouseButtonState & MK_XBUTTON2 ) != 0 );
		numSamples++;
	} 

};

void MouseWin::lockToWindow() {
	RECT screenRect;
	GetWindowRect(g_hWnd,&screenRect);
	ClipCursor(&screenRect);
	POINT pt;
	pt.x = (screenRect.right - screenRect.left) / 2;
	pt.y = (screenRect.bottom - screenRect.top) / 2;
	SetCursorPos( pt.x, pt.y );
}
void MouseWin::unlockFromWindow() {
	ClipCursor(NULL);
}
void MouseWin::showCursor() {
	ShowCursor( 1 );
}
void MouseWin::hideCursor() {
	ShowCursor( 0 );
}
void MouseWin::averageSamples() {
	POINT pt;
	GetPhysicalCursorPos( &pt );
	avgXPos = pt.x - midX;// / timeDelta;		
	avgYPos = pt.y - midY;// / timeDelta;
			
	SetPhysicalCursorPos( midX, midY );
}


}