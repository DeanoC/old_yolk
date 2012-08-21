//!-----------------------------------------------------
//!
//! \file development_context.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "mouse_win.h"

extern HWND g_hWnd;

namespace Core {

/**
MouseWin
*/
MouseWin::MouseWin()
{
}

MouseWin::~MouseWin() {
}

void MouseWin::update() {
//	lastXPos = xPos;
//	lastYPos = yPos;
}
/*
   
	// Consolidate the keyboard messages and pass them to the app's keyboard callback
	if( uMsg == WM_KEYDOWN ||
		uMsg == WM_SYSKEYDOWN ||
		uMsg == WM_KEYUP ||
		uMsg == WM_SYSKEYUP )
	{
		bool bKeyDown = ( uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN );
		DWORD dwMask = ( 1 << 29 );
		bool bAltDown = ( ( lParam & dwMask ) != 0 );

		bool* bKeys = GetDXUTState().GetKeys();
		bKeys[ ( BYTE )( wParam & 0xFF ) ] = bKeyDown;

		LPDXUTCALLBACKKEYBOARD pCallbackKeyboard = GetDXUTState().GetKeyboardFunc();
		if( pCallbackKeyboard )
			pCallbackKeyboard( ( UINT )wParam, bKeyDown, bAltDown, GetDXUTState().GetKeyboardFuncUserContext() );
	}*/
void MouseWin::processMouseMessages( UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	// Consolidate the mouse button messages and pass them to the app's mouse callback
/*	if( uMsg == WM_LBUTTONDOWN ||
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

		xPos = ( short )LOWORD( lParam );
		yPos = ( short )HIWORD( lParam );
		nMouseWheelDelta = 0;

		if( uMsg == WM_MOUSEWHEEL ) {
			// WM_MOUSEWHEEL passes screen mouse coords
			// so convert them to client coords
			POINT pt;
			pt.x = xPos; pt.y = yPos;
			ScreenToClient( g_hWnd, &pt );
			xPos = pt.x; yPos = pt.y;
			nMouseWheelDelta = ( short )HIWORD( wParam );
		}

		int nMouseButtonState = LOWORD( wParam );
		bLeftButton = ( ( nMouseButtonState & MK_LBUTTON ) != 0 );
		bRightButton = ( ( nMouseButtonState & MK_RBUTTON ) != 0 );
		bMiddleButton = ( ( nMouseButtonState & MK_MBUTTON ) != 0 );
		bSideButton1 = ( ( nMouseButtonState & MK_XBUTTON1 ) != 0 );
		bSideButton2 = ( ( nMouseButtonState & MK_XBUTTON2 ) != 0 );
	}
	*/
};

void MouseWin::lockToWindow() {
	RECT clientRect;
	GetWindowRect(g_hWnd,&clientRect);
	ClipCursor(&clientRect);
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

}