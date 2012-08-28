//!-----------------------------------------------------
//!
//! \file mouse_win.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "core/clock.h"
#include "mouse_win.h"
extern HWND g_hWnd;

//#define GetOurCursorPos( x ) GetPhysicalCursorPos( x )
//#define SetOurCursorPos( x, y ) SetPhysicalCursorPos( x, y )
#define GetOurCursorPos( x ) GetCursorPos( x )
#define SetOurCursorPos( x, y ) SetCursorPos( x, y )

namespace Core {

/**
MouseWin
*/
MouseWin::MouseWin() : cursorVisible( true ) {
//	lockToWindow();
	hideCursor();

	POINT pt;
	GetOurCursorPos( &pt );
	midX = pt.x;
	midY = pt.y;
	hasFocus = true;
}

MouseWin::~MouseWin() {
}

void MouseWin::update() {
	if( hasFocus ) {
		POINT pt;
		GetOurCursorPos( &pt );
		xPos = pt.x - midX;// / timeDelta;		
		yPos = pt.y - midY;// / timeDelta;
			
		SetOurCursorPos( midX, midY );
	}
}

void MouseWin::processMouseMessages( UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	hasFocus = (GetFocus() == g_hWnd);
	if( hasFocus == false ) {
		showCursor();
	} else {
		hideCursor();
	}
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
		uMsg == WM_MOUSEWHEEL ) {


		if( uMsg == WM_MOUSEWHEEL ) {
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
	if( !cursorVisible ) {
		ShowCursor( 1 );
		cursorVisible = true;
	}
}
void MouseWin::hideCursor() {
	if( cursorVisible ) {
		ShowCursor( 0 );
		cursorVisible = false;
	}
}

}