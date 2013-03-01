//!-----------------------------------------------------
//!
//! \file development_context.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "core/keyboard.h"
#include "keyboard_win.h"

namespace Core {

void KeyboardWinProcessKeyMessages( uint32_t message, uint16_t wParam, uint32_t lParam ) {
	if( Keyboard::exists() == false )
		return;

    // Consolidate the keyboard messages and pass them to the app's keyboard callback
    if( message == WM_KEYDOWN ||
        message == WM_SYSKEYDOWN ||
        message == WM_KEYUP ||
        message == WM_SYSKEYUP ) {

        bool bKeyDown = ( message == WM_KEYDOWN || message == WM_SYSKEYDOWN );
        DWORD dwMask = ( 1 << 29 );
        bool bAltDown = ( ( lParam & dwMask ) != 0 );

		if( bKeyDown ) {
	        Keyboard::get()->keyDataState[ ( wParam & 0xFF ) ] = (char)0x80;
		} else {
	        Keyboard::get()->keyDataState[ ( wParam & 0xFF ) ] = 0;
		}

		Keyboard::get()->keyDataState[ KT_LMENU ] = (char)0x80;
		Keyboard::get()->keyDataState[ KT_RMENU ] = (char)0x80;
    }
}

};