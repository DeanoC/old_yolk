//!-----------------------------------------------------
//!
//! \file keyboard_win.h
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_KEYBOARD_WIN_H
#define WIERD_CORE_KEYBOARD_WIN_H

#if defined( USE_DINPUT )
#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
#endif


namespace Core {

// each key has a label KT_x (i.e. A = KT_A
enum KeyTable {
#if defined( USE_DINPUT )

#define DIK_KEY_MAP(x) KT_##x = DIK_##x,
	DIK_KEY_MAP( ESCAPE )
	DIK_KEY_MAP( 1 )
	DIK_KEY_MAP( 2 )
	DIK_KEY_MAP( 3 )		
	DIK_KEY_MAP( 4 )
	DIK_KEY_MAP( 5 )
	DIK_KEY_MAP( 6 )
	DIK_KEY_MAP( 7 )
	DIK_KEY_MAP( 8 )
	DIK_KEY_MAP( 9 )
	DIK_KEY_MAP( 0 )
	DIK_KEY_MAP( MINUS )	// on main keyboard
	DIK_KEY_MAP( EQUALS )
	DIK_KEY_MAP( BACK )		// backspace
	DIK_KEY_MAP( TAB )
	DIK_KEY_MAP( Q )
	DIK_KEY_MAP( W )
	DIK_KEY_MAP( E )
	DIK_KEY_MAP( R )
	DIK_KEY_MAP( T )
	DIK_KEY_MAP( Y )
	DIK_KEY_MAP( U )
	DIK_KEY_MAP( I )
	DIK_KEY_MAP( O )
	DIK_KEY_MAP( P )
	DIK_KEY_MAP( LBRACKET )
	DIK_KEY_MAP( RBRACKET )
	DIK_KEY_MAP( RETURN )    /* Enter on main keyboard */
	DIK_KEY_MAP( LCONTROL )
	DIK_KEY_MAP( A )
	DIK_KEY_MAP( S )
	DIK_KEY_MAP( D )
	DIK_KEY_MAP( F )
	DIK_KEY_MAP( G )
	DIK_KEY_MAP( H )
	DIK_KEY_MAP( J )
	DIK_KEY_MAP( K )
	DIK_KEY_MAP( L )
	DIK_KEY_MAP( SEMICOLON )
	DIK_KEY_MAP( APOSTROPHE )
	DIK_KEY_MAP( GRAVE )    /* accent grave */
	DIK_KEY_MAP( LSHIFT )
	DIK_KEY_MAP( BACKSLASH )
	DIK_KEY_MAP( Z )
	DIK_KEY_MAP( X )
	DIK_KEY_MAP( C )
	DIK_KEY_MAP( V )
	DIK_KEY_MAP( B )
	DIK_KEY_MAP( N )
	DIK_KEY_MAP( M )
	DIK_KEY_MAP( COMMA )
	DIK_KEY_MAP( PERIOD )    /* . on main keyboard */
	DIK_KEY_MAP( SLASH )    /* / on main keyboard */
	DIK_KEY_MAP( RSHIFT )
	DIK_KEY_MAP( MULTIPLY )    /* * on numeric keypad */
	DIK_KEY_MAP( LMENU )    /* left Alt */
	DIK_KEY_MAP( SPACE )
	DIK_KEY_MAP( CAPITAL )
	DIK_KEY_MAP( F1 )
	DIK_KEY_MAP( F2 )
	DIK_KEY_MAP( F3 )
	DIK_KEY_MAP( F4 )
	DIK_KEY_MAP( F5 )
	DIK_KEY_MAP( F6 )
	DIK_KEY_MAP( F7 )
	DIK_KEY_MAP( F8 )
	DIK_KEY_MAP( F9 )
	DIK_KEY_MAP( F10 )
	DIK_KEY_MAP( NUMLOCK )
	DIK_KEY_MAP( SCROLL )    /* Scroll Lock */
	DIK_KEY_MAP( NUMPAD7 )
	DIK_KEY_MAP( NUMPAD8 )
	DIK_KEY_MAP( NUMPAD9 )
	DIK_KEY_MAP( SUBTRACT )    /* - on numeric keypad */
	DIK_KEY_MAP( NUMPAD4 )
	DIK_KEY_MAP( NUMPAD5 )
	DIK_KEY_MAP( NUMPAD6 )
	DIK_KEY_MAP( ADD )    /* + on numeric keypad */
	DIK_KEY_MAP( NUMPAD1 )
	DIK_KEY_MAP( NUMPAD2 )
	DIK_KEY_MAP( NUMPAD3 )
	DIK_KEY_MAP( NUMPAD0 )
	DIK_KEY_MAP( DECIMAL )    /* . on numeric keypad */
	DIK_KEY_MAP( OEM_102 )    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
	DIK_KEY_MAP( F11 )
	DIK_KEY_MAP( F12 )
	DIK_KEY_MAP( F13 )    /*                     (NEC PC98) */
	DIK_KEY_MAP( F14 )    /*                     (NEC PC98) */
	DIK_KEY_MAP( F15 )    /*                     (NEC PC98) */
	DIK_KEY_MAP( KANA )    /* (Japanese keyboard)            */
	DIK_KEY_MAP( ABNT_C1 )    /* /? on Brazilian keyboard */
	DIK_KEY_MAP( CONVERT )    /* (Japanese keyboard)            */
	DIK_KEY_MAP( NOCONVERT )    /* (Japanese keyboard)            */
	DIK_KEY_MAP( YEN )    /* (Japanese keyboard)            */
	DIK_KEY_MAP( ABNT_C2 )    /* Numpad . on Brazilian keyboard */
	DIK_KEY_MAP( NUMPADEQUALS )    /* = on numeric keypad (NEC PC98) */
	DIK_KEY_MAP( PREVTRACK )    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
	DIK_KEY_MAP( AT )    /*                     (NEC PC98) */
	DIK_KEY_MAP( COLON )    /*                     (NEC PC98) */
	DIK_KEY_MAP( UNDERLINE )    /*                     (NEC PC98) */
	DIK_KEY_MAP( KANJI )    /* (Japanese keyboard)            */
	DIK_KEY_MAP( STOP )    /*                     (NEC PC98) */
	DIK_KEY_MAP( AX )    /*                     (Japan AX) */
	DIK_KEY_MAP( UNLABELED ) /*                        (J3100) */
	DIK_KEY_MAP( NEXTTRACK )    /* Next Track */
	DIK_KEY_MAP( NUMPADENTER )    /* Enter on numeric keypad */
	DIK_KEY_MAP( RCONTROL )
	DIK_KEY_MAP( MUTE )    /* Mute */
	DIK_KEY_MAP( CALCULATOR )    /* Calculator */
	DIK_KEY_MAP( PLAYPAUSE )    /* Play / Pause */
	DIK_KEY_MAP( MEDIASTOP )    /* Media Stop */
	DIK_KEY_MAP( VOLUMEDOWN )    /* Volume - */
	DIK_KEY_MAP( VOLUMEUP )    /* Volume + */
	DIK_KEY_MAP( WEBHOME )    /* Web home */
	DIK_KEY_MAP( NUMPADCOMMA )    /* , on numeric keypad (NEC PC98) */
	DIK_KEY_MAP( DIVIDE )    /* / on numeric keypad */
	DIK_KEY_MAP( SYSRQ )
	DIK_KEY_MAP( RMENU )    /* right Alt */
	DIK_KEY_MAP( PAUSE )    /* Pause */
	DIK_KEY_MAP( HOME )    /* Home on arrow keypad */
	DIK_KEY_MAP( UP )    /* UpArrow on arrow keypad */
	DIK_KEY_MAP( PRIOR )    /* PgUp on arrow keypad */
	DIK_KEY_MAP( LEFT )    /* LeftArrow on arrow keypad */
	DIK_KEY_MAP( RIGHT )    /* RightArrow on arrow keypad */
	DIK_KEY_MAP( END )    /* End on arrow keypad */
	DIK_KEY_MAP( DOWN )    /* DownArrow on arrow keypad */
	DIK_KEY_MAP( NEXT )    /* PgDn on arrow keypad */
	DIK_KEY_MAP( INSERT )    /* Insert on arrow keypad */
	DIK_KEY_MAP( DELETE )    /* Delete on arrow keypad */
	DIK_KEY_MAP( LWIN )    /* Left Windows key */
	DIK_KEY_MAP( RWIN )    /* Right Windows key */
	DIK_KEY_MAP( APPS )    /* AppMenu key */
	DIK_KEY_MAP( POWER )    /* System Power */
	DIK_KEY_MAP( SLEEP )    /* System Sleep */
	DIK_KEY_MAP( WAKE )    /* System Wake */
	DIK_KEY_MAP( WEBSEARCH )    /* Web Search */
	DIK_KEY_MAP( WEBFAVORITES )    /* Web Favorites */
	DIK_KEY_MAP( WEBREFRESH )    /* Web Refresh */
	DIK_KEY_MAP( WEBSTOP )    /* Web Stop */
	DIK_KEY_MAP( WEBFORWARD )    /* Web Forward */
	DIK_KEY_MAP( WEBBACK )    /* Web Back */
	DIK_KEY_MAP( MYCOMPUTER )    /* My Computer */
	DIK_KEY_MAP( MAIL )    /* Mail */
	DIK_KEY_MAP( MEDIASELECT )    /* Media Select */

#undef DIK_KEY_MAP
#else

#define VK_KEY_MAP(x) KT_##x = VK_##x,
#define NK_KEY_MAP(c , d) KT_##c = d,
	VK_KEY_MAP( ESCAPE )
	NK_KEY_MAP( 1, '1' )
	NK_KEY_MAP( 2, '2' )
	NK_KEY_MAP( 3, '3' )
	NK_KEY_MAP( 4, '4' )
	NK_KEY_MAP( 5, '5' )
	NK_KEY_MAP( 6, '6' )
	NK_KEY_MAP( 7, '7' )
	NK_KEY_MAP( 8, '8' )
	NK_KEY_MAP( 9, '9' )
	NK_KEY_MAP( 0, '0' )
	KT_MINUS = VK_OEM_MINUS,	// _-on main keyboard
	KT_EQUALS = VK_OEM_PLUS,	// += on main keyboard
	VK_KEY_MAP( BACK )		// backspace
	VK_KEY_MAP( TAB )
	NK_KEY_MAP( Q, 'Q' )
	NK_KEY_MAP( W, 'W' )
	NK_KEY_MAP( E, 'E' )
	NK_KEY_MAP( R, 'R' )
	NK_KEY_MAP( T, 'T' )
	NK_KEY_MAP( Y, 'Y' )
	NK_KEY_MAP( U, 'U' )
	NK_KEY_MAP( I, 'I' )
	NK_KEY_MAP( O, 'O' )
	NK_KEY_MAP( P, 'P' )
	KT_LBRACKET = VK_OEM_4,
	KT_RBRACKET = VK_OEM_5,
	VK_KEY_MAP( RETURN )    /* Enter on main keyboard */
	VK_KEY_MAP( LCONTROL )
	NK_KEY_MAP( A, 'A' )
	NK_KEY_MAP( S, 'S' )
	NK_KEY_MAP( D, 'D' )
	NK_KEY_MAP( F, 'F' )
	NK_KEY_MAP( G, 'G' )
	NK_KEY_MAP( H, 'H' )
	NK_KEY_MAP( J, 'J' )
	NK_KEY_MAP( K, 'K' )
	NK_KEY_MAP( L, 'L' )
	KT_SEMICOLON = VK_OEM_1,
	KT_APOSTROPHE = VK_OEM_3,
	KT_GRAVE = VK_OEM_2,
	VK_KEY_MAP( LSHIFT )
	KT_BACKSLASH = VK_OEM_5,
	NK_KEY_MAP( Z, 'Z' )
	NK_KEY_MAP( X, 'X' )
	NK_KEY_MAP( C, 'C' )
	NK_KEY_MAP( V, 'V' )
	NK_KEY_MAP( B, 'B' )
	NK_KEY_MAP( N, 'N' )
	NK_KEY_MAP( M, 'M' )
	KT_COMMA = VK_OEM_COMMA,
	KT_PEROID = VK_OEM_PERIOD,
	KT_SLASH = VK_OEM_2,
	VK_KEY_MAP( RSHIFT )
	VK_KEY_MAP( MULTIPLY )    /* * on numeric keypad */
	VK_KEY_MAP( LMENU )    /* left Alt */
	VK_KEY_MAP( SPACE )
	VK_KEY_MAP( CAPITAL )
	VK_KEY_MAP( F1 )
	VK_KEY_MAP( F2 )
	VK_KEY_MAP( F3 )
	VK_KEY_MAP( F4 )
	VK_KEY_MAP( F5 )
	VK_KEY_MAP( F6 )
	VK_KEY_MAP( F7 )
	VK_KEY_MAP( F8 )
	VK_KEY_MAP( F9 )
	VK_KEY_MAP( F10 )
	VK_KEY_MAP( NUMLOCK )
	VK_KEY_MAP( SCROLL )    /* Scroll Lock */
	VK_KEY_MAP( NUMPAD7 )
	VK_KEY_MAP( NUMPAD8 )
	VK_KEY_MAP( NUMPAD9 )
	VK_KEY_MAP( SUBTRACT )    /* - on numeric keypad */
	VK_KEY_MAP( NUMPAD4 )
	VK_KEY_MAP( NUMPAD5 )
	VK_KEY_MAP( NUMPAD6 )
	VK_KEY_MAP( ADD )    /* + on numeric keypad */
	VK_KEY_MAP( NUMPAD1 )
	VK_KEY_MAP( NUMPAD2 )
	VK_KEY_MAP( NUMPAD3 )
	VK_KEY_MAP( NUMPAD0 )
	VK_KEY_MAP( DECIMAL )    /* . on numeric keypad */
	VK_KEY_MAP( OEM_102 )    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
	VK_KEY_MAP( F11 )
	VK_KEY_MAP( F12 )
	VK_KEY_MAP( F13 )    /*                     (NEC PC98) */
	VK_KEY_MAP( F14 )    /*                     (NEC PC98) */
	VK_KEY_MAP( F15 )    /*                     (NEC PC98) */
	VK_KEY_MAP( KANA )    /* (Japanese keyboard)            */
	KT_ABNT_C1 = 0xC1,/* /? on Brazilian keyboard */
	VK_KEY_MAP( CONVERT )    /* (Japanese keyboard)            */
	KT_NOCONVERT = VK_NONCONVERT, /* (Japanese keyboard)            */
	//VK_KEY_MAP( YEN )    /* (Japanese keyboard)            */
	KT_ABNT_C2 = 0xC2,	/* Numpad . on Brazilian keyboard */
	//VK_KEY_MAP( NUMPADEQUALS )    /* = on numeric keypad (NEC PC98) */
	KT_PREVTRACK = VK_MEDIA_PREV_TRACK,
	//VK_KEY_MAP( PREVTRACK )    /* Previous Track (VK_CIRCUMFLEX on Japanese keyboard) */
	//VK_KEY_MAP( AT )    /*                     (NEC PC98) */
	//VK_KEY_MAP( COLON )    /*                     (NEC PC98) */
	//VK_KEY_MAP( UNDERLINE )    /*                     (NEC PC98) */
	VK_KEY_MAP( KANJI )    /* (Japanese keyboard)            */
	KT_STOP = VK_MEDIA_STOP,
	KT_AX = VK_OEM_AX,		/*                     (Japan AX) */
	//VK_KEY_MAP( UNLABELED ) /*                        (J3100) */
	KT_NEXTTRACK = VK_MEDIA_NEXT_TRACK, /* Next Track */
	//VK_KEY_MAP( NUMPADENTER )    /* Enter on numeric keypad */
	VK_KEY_MAP( RCONTROL )
	KT_MUTE = VK_VOLUME_MUTE,	/* Mute */
	//VK_KEY_MAP( CALCULATOR )    /* Calculator */
	KT_PLAYPAUSE = VK_MEDIA_PLAY_PAUSE, /* Play / Pause */
	KT_MEDIASTOP = VK_MEDIA_STOP,	/* Media Stop */
	KT_VOLUMEDOWN = VK_VOLUME_DOWN, /* Volume - */
	KT_VOLUMEUP = VK_VOLUME_UP,		/* Volume + */
	KT_WEBHOME = VK_BROWSER_HOME,	/* Web home */
	//VK_KEY_MAP( NUMPADCOMMA )    /* , on numeric keypad (NEC PC98) */
	VK_KEY_MAP( DIVIDE )    /* / on numeric keypad */
	KT_SYSRQ = VK_SNAPSHOT,
	VK_KEY_MAP( RMENU )    /* right Alt */
	VK_KEY_MAP( PAUSE )    /* Pause */
	VK_KEY_MAP( HOME )    /* Home on arrow keypad */
	VK_KEY_MAP( UP )    /* UpArrow on arrow keypad */
	VK_KEY_MAP( PRIOR )    /* PgUp on arrow keypad */
	VK_KEY_MAP( LEFT )    /* LeftArrow on arrow keypad */
	VK_KEY_MAP( RIGHT )    /* RightArrow on arrow keypad */
	VK_KEY_MAP( END )    /* End on arrow keypad */
	VK_KEY_MAP( DOWN )    /* DownArrow on arrow keypad */
	VK_KEY_MAP( NEXT )    /* PgDn on arrow keypad */
	VK_KEY_MAP( INSERT )    /* Insert on arrow keypad */
	VK_KEY_MAP( DELETE )    /* Delete on arrow keypad */
	VK_KEY_MAP( LWIN )    /* Left Windows key */
	VK_KEY_MAP( RWIN )    /* Right Windows key */
	VK_KEY_MAP( APPS )    /* AppMenu key */
	//VK_KEY_MAP( POWER )    /* System Power */
	VK_KEY_MAP( SLEEP )    /* System Sleep */
	//VK_KEY_MAP( WAKE )    /* System Wake */
	KT_WEBSEARCH = VK_BROWSER_SEARCH, /* Web Search */
	KT_WEBFAVORITES= VK_BROWSER_FAVORITES,/* Web Favorites */
	KT_WEBREFRESH = VK_BROWSER_REFRESH,/* Web Refresh */
	KT_WEBSTOP = VK_BROWSER_STOP,/* Web Stop */
	KT_WEBFORWARD = VK_BROWSER_FORWARD, /* Web Forward */
	KT_WEBBACK = VK_BROWSER_BACK, /* Web Back */
	//VK_KEY_MAP( MYCOMPUTER )    /* My Computer */
	KT_MAIL = VK_LAUNCH_MEDIA_SELECT, /* Mail */
	KT_MEDIASELECT = VK_LAUNCH_MEDIA_SELECT, /* Media Select */

#undef VK_KEY_MAP
#undef NK_KEY_MAP
#endif
};


class KeyboardWin : public Singleton<KeyboardWin> {
public:
	KeyboardWin( bool fullscreen = false /*TODO*/ );
	~KeyboardWin();

	void update();

	bool keyDown( KeyTable key ) {	
		return ((m_KeyData[ key & 0xFF ] & 0x80) != 0);
	}

	bool keyUp( KeyTable key ) {	
		return (m_KeyData[ key & 0xFF ] == 0);
	}

	//! is the key being held (has it been down for at least a few calls)
	bool keyHeld( KeyTable key ) {
		if( keyDown(key) ) {
			if ( !m_KeyHeld[ key & 0xFF ] ) {
				m_KeyHeld[ key & 0xFF ] = true;
				return false;
			} else {
				return true;
			}
		} else {
			m_KeyHeld[ key & 0xFF ] = false;
			return false;
		}
	}

	// key down only once per press
	bool keyDownOnce( KeyTable key ) {
		// note the short circuit order is important here don't rearrange
		return (!keyHeld( key ) && keyDown( key ));
	}

	void processKeyMessages( UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
	static const int MAX_CHARS = 256;

protected:
#if defined( USE_DINPUT )
	LPDIRECTINPUT8			m_diDevice;
	LPDIRECTINPUTDEVICE8A	m_diKeyboard;
#endif
	char					m_KeyData[ MAX_CHARS ];
	bool					m_KeyHeld[ MAX_CHARS ];

};


}	//namespace Core


#endif
