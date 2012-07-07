#include "core/core.h"
#include "core/sysmsg.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
 
#ifdef KeyPress
const int XKeyPress   = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease
#endif

#include "keyboard_x11.h"

Display* g_x11display = nullptr;

namespace Core {
	void X11PumpEvents() {

		XEvent xev;
		if( g_x11display == nullptr ) {
			return;
		}

		while( XPending(g_x11display) > 0 ) {
			XNextEvent(g_x11display, &xev);
			switch( xev.type ) {
			case Expose: {
				XWindowAttributes gwa;				
        		XGetWindowAttributes(g_x11display, ((XExposeEvent&)xev).window, &gwa);
				Core::SystemMessage::get()->resize( gwa.width, gwa.height );
        		break;
        	}
//        	case Quit: 
//				Core::SystemMessage::get()->quit();
//        		break;
			case XKeyPress:
			case XKeyRelease:
				KeyboardX11ProcessKeyEvent( (xev.type == XKeyPress) ? true : false, (XKeyEvent*) &xev );
				break;
			}
		}
	}
}