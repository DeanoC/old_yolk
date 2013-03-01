#ifndef _YOLK_CORE_KEYBOARD_H_
#define _YOLK_CORE_KEYBOARD_H_ 1

#if PLATFORM == WINDOWS
#include "platform_windows/keyboard_win.h"
#elif PLATFORM == POSIX
#include "platform_posix/keyboard_x11.h"
#endif

namespace Core {

class Keyboard : public Singleton<Keyboard> {
public:
#if PLATFORM == WINDOWS
	friend void KeyboardWinProcessKeyMessages( uint32_t message, uint16_t wParam, uint32_t lParam );
#elif PLATFORM == POSIX
	friend void KeyboardX11ProcessKeyEvent( bool down, XKeyEvent* event );
	#endif
	static const int MAX_KEY_CHARS = 256;

	Keyboard() {
		memset( keyDataState, 0, sizeof(char) * MAX_KEY_CHARS );
		memset( keyHeldState, 0, sizeof(bool) * MAX_KEY_CHARS );
	}

	bool keyDown( KeyTable key ) {	
		return ((keyDataState[ key & 0xFF ] & 0x80) != 0);
	}

	bool keyUp( KeyTable key ) {	
		return (keyDataState[ key & 0xFF ] == 0);
	}

	//! is the key being held (has it been down for at least a few calls)
	bool keyHeld( KeyTable key ) {
		if( keyDown(key) ) {
			if ( !keyHeldState[ key & 0xFF ] ) {
				keyHeldState[ key & 0xFF ] = true;
				return false;
			} else {
				return true;
			}
		} else {
			keyHeldState[ key & 0xFF ] = false;
			return false;
		}
	}

	// key down only once per press
	bool keyDownOnce( KeyTable key ) {
		// note the short circuit order is important here don't rearrange
		return (!keyHeld( key ) && keyDown( key ));
	}

protected:
	char					keyDataState[ MAX_KEY_CHARS ];
	bool					keyHeldState[ MAX_KEY_CHARS ];
};


}

#endif