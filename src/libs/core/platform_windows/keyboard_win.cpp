//!-----------------------------------------------------
//!
//! \file development_context.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "keyboard_win.h"

#if defined( USE_DINPUT )

extern HWND g_hWnd;
#pragma comment( lib, "dinput8" )
#pragma comment( lib, "dxguid" )

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

#endif

namespace Core {
#if defined( USE_DINPUT )

/**
KeyboardWin
*/
KeyboardWin::KeyboardWin( bool fullscreen ) :
	m_diDevice(0),
	m_diKeyboard(0) {
	HRESULT         hr; 

	// create di
	hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
							IID_IDirectInput8, (void**)&m_diDevice, NULL ); 
	DXFAIL(hr);

	// create mouse device
	hr = m_diDevice->CreateDevice(GUID_SysKeyboard, &m_diKeyboard, NULL);
	DXFAIL(hr);

	// set the data format to standard mouse format
	hr = m_diKeyboard->SetDataFormat(&c_dfDIKeyboard);
	DXFAIL(hr);

	DWORD flags;

	if(fullscreen)
		flags = DISCL_FOREGROUND | DISCL_EXCLUSIVE;
	else
		flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

	// monitor input in the background (slight dodgy but bloody hWnd's)
	hr = m_diKeyboard->SetCooperativeLevel(g_hWnd, flags /*| DISCL_NOWINKEY*/);
	DXFAIL(hr);

	// Acquire the newly created device
	m_diKeyboard->Acquire();

	ZeroMemory( m_KeyData, sizeof(m_KeyData) );
	ZeroMemory( m_KeyHeld, sizeof(m_KeyHeld) );

}

KeyboardWin::~KeyboardWin() {
	if( m_diKeyboard ) 
		m_diKeyboard->Unacquire();

	SAFE_RELEASE(m_diKeyboard);
	SAFE_RELEASE(m_diDevice);
}

void KeyboardWin::Update()
{
	HRESULT         hr; 

	// Get the input's device state, and put the state in dims
	ZeroMemory( m_KeyData, sizeof(m_KeyData) );
	hr = m_diKeyboard->GetDeviceState( sizeof(m_KeyData), m_KeyData );
	if( FAILED(hr) ) 
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.
		// We just re-acquire and try again.
		
		// If input is lost then acquire and keep trying 
		hr = m_diKeyboard->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = m_diKeyboard->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		ZeroMemory( m_KeyData, sizeof(m_KeyData) );
	}
}
#else
KeyboardWin::KeyboardWin( bool fullscreen ) {
	ZeroMemory( m_KeyData, sizeof(m_KeyData) );
	ZeroMemory( m_KeyHeld, sizeof(m_KeyHeld) );
}

KeyboardWin::~KeyboardWin() {
}

void KeyboardWin::update() {
}

void KeyboardWin::processKeyMessages( UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    // Consolidate the keyboard messages and pass them to the app's keyboard callback
    if( uMsg == WM_KEYDOWN ||
        uMsg == WM_SYSKEYDOWN ||
        uMsg == WM_KEYUP ||
        uMsg == WM_SYSKEYUP ) {
        bool bKeyDown = ( uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN );
        DWORD dwMask = ( 1 << 29 );
        bool bAltDown = ( ( lParam & dwMask ) != 0 );

		if( bKeyDown ) {
	        m_KeyData[ ( wParam & 0xFF ) ] = (char)0x80;
		} else {
	        m_KeyData[ ( wParam & 0xFF ) ] = 0;
		}

		m_KeyData[ KT_LMENU ] = (char)0x80;
		m_KeyData[ KT_RMENU ] = (char)0x80;
    }
}

#endif
};