#ifndef _YOLK_CORE_PLATFORM_POSIX_KEYBOARD_X11_H_
#define _YOLK_CORE_PLATFORM_POSIX_KEYBOARD_X11_H_ 1

#include "X11/Xlib.h"
#include "X11/keysym.h"

namespace Core {

// each key has a label KT_x (i.e. A = KT_A
enum KeyTable {
#define NK_KEY_MAP(c , d) KT_##c = d,
#define KT_MAP(c, d) KT_##c = (XK_##d & 0x00FF),
#define KT_F_MAP(c) KT_##c = (XK_##c & 0x00FF),
#define KT_KP_MAP(c, d) KT_##c = (XK_KP_##d & 0x00FF),
	KT_MAP( ESCAPE, Escape )
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
	NK_KEY_MAP( MINUS, '_' ) // _-on main keyboard
	NK_KEY_MAP( EQUALS, '+' ) // += on main keyboard
	KT_MAP( BACK, BackSpace )		// backspace
	KT_MAP( TAB, Tab )
	NK_KEY_MAP( Q, 'q' )
	NK_KEY_MAP( W, 'w' )
	NK_KEY_MAP( E, 'e' )
	NK_KEY_MAP( R, 'r' )
	NK_KEY_MAP( T, 't' )
	NK_KEY_MAP( Y, 'y' )
	NK_KEY_MAP( U, 'u' )
	NK_KEY_MAP( I, 'i' )
	NK_KEY_MAP( O, 'o' )
	NK_KEY_MAP( P, 'p' )
	NK_KEY_MAP( LBRACKET, '[' )
	NK_KEY_MAP( R_BRACKET, ']' )
	KT_MAP( RETURN, Return )    /* Enter on main keyboard */
	KT_MAP( LCONTROL, Control_L)
	NK_KEY_MAP( A, 'a' )
	NK_KEY_MAP( S, 's' )
	NK_KEY_MAP( D, 'd' )
	NK_KEY_MAP( F, 'f' )
	NK_KEY_MAP( G, 'g' )
	NK_KEY_MAP( H, 'h' )
	NK_KEY_MAP( J, 'j' )
	NK_KEY_MAP( K, 'k' )
	NK_KEY_MAP( L, 'l' )
	NK_KEY_MAP( SEMICOLON, ';' )
	NK_KEY_MAP( APOSTROPHE, '\'' )
	KT_MAP( LSHIFT, Shift_L )
	NK_KEY_MAP( BACKSLASH, '\\' )
	NK_KEY_MAP( Z, 'z' )
	NK_KEY_MAP( X, 'x' )
	NK_KEY_MAP( C, 'c' )
	NK_KEY_MAP( V, 'v' )
	NK_KEY_MAP( B, 'b' )
	NK_KEY_MAP( N, 'n' )
	NK_KEY_MAP( M, 'm' )
	NK_KEY_MAP( COMMA, ',' )
	NK_KEY_MAP( PEROID, '.' )
	NK_KEY_MAP( SLASH, '/' )
	KT_MAP( RSHIFT, Shift_R )

	KT_KP_MAP( MULTIPLY, Multiply ) /* * on numeric keypad */
	KT_MAP( LALT, Alt_L )

	KT_KP_MAP( SPACE, Space )
//	VK_KEY_MAP( CAPITAL )
	KT_F_MAP( F1 )
	KT_F_MAP( F2 )
	KT_F_MAP( F3 )
	KT_F_MAP( F4 )
	KT_F_MAP( F5 )
	KT_F_MAP( F6 )
	KT_F_MAP( F7 )
	KT_F_MAP( F8 )
	KT_F_MAP( F9 )
	KT_F_MAP( F10 )
	KT_MAP( NUMLOCK, Num_Lock )
	KT_MAP( SCROLL, Scroll_Lock ) /* Scroll Lock */
	KT_KP_MAP( NUMPAD7, 7 )
	KT_KP_MAP( NUMPAD8, 8 )
	KT_KP_MAP( NUMPAD9, 9 )
	KT_KP_MAP( SUBTRACT, Subtract )    /* - on numeric keypad */
	KT_KP_MAP( NUMPAD4, 4 )
	KT_KP_MAP( NUMPAD5, 5 )
	KT_KP_MAP( NUMPAD6, 6 )
	KT_KP_MAP( ADD, Add )    /* + on numeric keypad */
	KT_KP_MAP( NUMPAD1, 1 )
	KT_KP_MAP( NUMPAD2, 2 )
	KT_KP_MAP( NUMPAD3, 3 )
	KT_KP_MAP( NUMPAD0, 0 )
	KT_KP_MAP( DECIMAL, Decimal )    /* . on numeric keypad */
	KT_F_MAP( F11 )
	KT_F_MAP( F12 )
	KT_F_MAP( F13 )    /*                     (NEC PC98) */
	KT_F_MAP( F14 )    /*                     (NEC PC98) */
	KT_F_MAP( F15 )    /*                     (NEC PC98) */
	KT_KP_MAP( NUMPADENTER, Enter )    /* Enter on numeric keypad */
	KT_MAP( RCONTROL, Control_R )
	KT_KP_MAP( DIVIDE, Divide )    /* / on numeric keypad */
	KT_MAP( RALT, Alt_R )    /* right Alt */
	KT_MAP( PAUSE, Pause )    /* Pause */
	KT_MAP( HOME, Home )    /* Home on arrow keypad */
	KT_MAP( UP, Up )    /* UpArrow on arrow keypad */
	KT_MAP( PGUP, Page_Up )    /* PgUp on arrow keypad */
	KT_MAP( LEFT, Left )    /* LeftArrow on arrow keypad */
	KT_MAP( RIGHT, Right )    /* RightArrow on arrow keypad */
	KT_MAP( END, End )    /* End on arrow keypad */
	KT_MAP( DOWN, Down )    /* DownArrow on arrow keypad */
	KT_MAP( PGDN, Page_Down )    /* PgDn on arrow keypad */
	KT_MAP( INSERT, Insert )    /* Insert on arrow keypad */
	KT_MAP( DELETE, Delete )    /* Delete on arrow keypad */
//	KT_MAP( LWIN )    /* Left Windows key */
//	KT_MAP( RWIN )    /* Right Windows key */

#undef NK_KEY_MAP
#undef KT_MAP
#undef KT_F_MAP
#undef KT_KP_MAP
};

extern void KeyboardX11ProcessKeyEvent( bool down, XKeyEvent* event );

}


#endif
