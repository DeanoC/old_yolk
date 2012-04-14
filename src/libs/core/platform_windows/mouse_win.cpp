//!-----------------------------------------------------
//!
//! \file development_context.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "mouse_win.h"

extern HWND g_hWnd;

#if defined( USE_DINPUT )

// debug helpers 
DECLARE_EXCEPTION(FatalDirectX, Fatal DirectX error occured);

// these shouldn't really be here, but graphics.h defines them
// but they aren't availible to core... need a think
#if DEBUG_LEVEL >= DEBUG_NONE
#include <dxerr.h>
/// DXFAIL requires HRESULT hr to be in scope
#	define DXFAIL(x)	if( FAILED(hr = x) ) { DXTRACE_ERR_MSGBOX(0,hr ); CoreThrowException( FatalDirectX, "" ); }
#else
/// DXFAIL requires HRESULT hr to be in scope
#	define DXFAIL(x) (hr = x)
#endif

#pragma comment( lib, "dinput8" )
#pragma comment( lib, "dxguid" )
#pragma comment( lib, "dxerr" )

#endif

namespace Core {

/**
MouseWin
*/
MouseWin::MouseWin()
#if !defined( USE_DINPUT )
:	bLeftButton( false ),
	bRightButton( false ),
	bMiddleButton( false ),
	bSideButton1( false ),
	bSideButton2( false ),
	nMouseWheelDelta( 0 ),
	xPos( 0 ),
	yPos( 0 ),
	lastXPos( 0 ),
	lastYPos( 0 ) 
#endif
{
#if defined( USE_DINPUT )
	HRESULT hr;
	// create di
	hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
							IID_IDirectInput8, (void**)&m_diDevice, NULL ); 
	DXFAIL(hr);

	// create mouse device
	hr = m_diDevice->CreateDevice(GUID_SysMouse, &m_diMouse, NULL);
	DXFAIL(hr);

	// set the data format to standard mouse format
	hr = m_diMouse->SetDataFormat(&c_dfDIMouse2);
	DXFAIL(hr);

	DWORD flags;

//	if(fullscreen)
//		flags = DISCL_FOREGROUND | DISCL_EXCLUSIVE;
//	else
		flags = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
//		flags = DISCL_FOREGROUND | DISCL_EXCLUSIVE;

	// monitor input in the background (slight dodgy but bloody hWnd's)
	hr = m_diMouse->SetCooperativeLevel(g_hWnd, flags /*| DISCL_NOWINKEY*/);
	DXFAIL(hr);

	// Acquire the newly created device
	m_diMouse->Acquire();
	ZeroMemory( &m_mouseData, sizeof(m_mouseData) );
#endif

}

MouseWin::~MouseWin() {
#if defined( USE_DINPUT )
	if( m_diMouse ) 
		m_diMouse->Unacquire();

	SAFE_RELEASE(m_diMouse);
	SAFE_RELEASE(m_diDevice);
#endif
}

void MouseWin::update() {
#if !defined( USE_DINPUT )
	lastXPos = xPos;
	lastYPos = yPos;
#else
	HRESULT         hr; 

	// Get the input's device state, and put the state in dims
	ZeroMemory( &m_mouseData, sizeof(m_mouseData) );
	hr = m_diMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &m_mouseData );
	if( FAILED(hr) ) 
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.
		// We just re-acquire and try again.
		
		// If input is lost then acquire and keep trying 
		hr = m_diMouse->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = m_diMouse->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		ZeroMemory( &m_mouseData, sizeof(m_mouseData) );
	}
#endif
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
#if !defined( USE_DINPUT )
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
#endif
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