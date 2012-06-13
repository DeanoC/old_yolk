//!-----------------------------------------------------
//!
//! \file development_context.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "core/keyboard.h"
#include "keyboard_win.h"

namespace Core {

void KeyboardWinProcessKeyMessages( UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	if( Keyboard::exists() == false )
		return;

    // Consolidate the keyboard messages and pass them to the app's keyboard callback
    if( uMsg == WM_KEYDOWN ||
        uMsg == WM_SYSKEYDOWN ||
        uMsg == WM_KEYUP ||
        uMsg == WM_SYSKEYUP ) {

        bool bKeyDown = ( uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN );
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