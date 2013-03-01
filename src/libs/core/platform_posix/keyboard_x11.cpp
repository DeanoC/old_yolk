//!-----------------------------------------------------
//!
//! \file keyboard_x11.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "core/keyboard.h"
#include "keyboard_x11.h"

namespace Core {

void KeyboardX11ProcessKeyEvent( bool down, XKeyEvent* event ) {

	uint32_t sym = (uint32_t) XLookupKeysym(event, 0);
	if( down ) {
        Keyboard::get()->keyDataState[ ( sym & 0xFF ) ] = (char)0x80;
	} else {
        Keyboard::get()->keyDataState[ ( sym & 0xFF ) ] = 0;
	}

}

}