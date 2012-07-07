#include "gl/gl.h"
#include "glxew.h"
#include "scene/rendercontext.h"
#include "gl/rendercontext.h"
#include "gl/gfx.h"

extern Display* g_x11display;

static const int visual_attribs[] = {
	GLX_X_RENDERABLE    , True,
	GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
	GLX_RENDER_TYPE     , GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
	GLX_RED_SIZE        , 8,
	GLX_GREEN_SIZE      , 8,
	GLX_BLUE_SIZE       , 8,
	GLX_ALPHA_SIZE      , 0,
	GLX_DEPTH_SIZE      , 0,
	GLX_STENCIL_SIZE    , 0,
	GLX_DOUBLEBUFFER    , True,
	None
};
static const int context_attribs[] = {
	GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
	GLX_CONTEXT_MINOR_VERSION_ARB, 2,
//	GLX_CONTEXT_FLAGS_ARB, 	GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  | GLX_CONTEXT_DEBUG_BIT_ARB, 
//	GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
	None
};

extern "C" GLenum glxewContextInit (void);

namespace Gl {

bool Gfx::createGLContext() {

	Display *display = XOpenDisplay(0);

	if ( !display ) {
		return false;
	}

	GLenum errglx = glxewContextInit();
	if (GLEW_OK != errglx)
		return  false;

	x11Display = g_x11display = display;

	int glx_major, glx_minor;

	// FBConfigs were added in GLX version 1.3.
	if ( !glXQueryVersion( display, &glx_major, &glx_minor ) || 
	   ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) ) {
		return false;
	}

	int fbcount;
	GLXFBConfig *fbc = glXChooseFBConfig( display, DefaultScreen( display ), 
													visual_attribs, &fbcount );
	if ( !fbc ) {
		return false;
	}

	// first valid is fine by me
	GLXFBConfig bestFbc = fbc[ 0 ];

	// Be sure to free the FBConfig list allocated by glXChooseFBConfig()
	XFree( fbc );

	// Get a visual
	XVisualInfo *vi = glXGetVisualFromFBConfig( display, bestFbc );

	XSetWindowAttributes swa;
	Colormap cmap;
	swa.colormap = cmap = XCreateColormap( display,
	                                     RootWindow( display, vi->screen ), 
	                                     vi->visual, AllocNone );
	swa.background_pixmap = None ;
	swa.border_pixel      = 0;
	swa.event_mask        = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                            StructureNotifyMask;

	Window win = XCreateWindow( display, RootWindow( display, vi->screen ), 
	                          0, 0, screenWidth, screenHeight, 0, vi->depth, InputOutput, 
	                          vi->visual, 
	                          CWBorderPixel|CWColormap|CWEventMask, &swa );
	if ( !win ) {
		return false;
	}
	x11Window = win;

	// Done with the visual info data
	XFree( vi );

	XStoreName( display, win, "GL Window" );

	XMapWindow( display, win );

	GLXContext ctx = glXCreateContextAttribsARB( (Display*) x11Display, bestFbc, 0,
			                          True, context_attribs );

	glXMakeCurrent( (Display*) x11Display, x11Window, ctx );

	GLenum err = glewInit();
	if (GLEW_OK != err)
		return  false;

	// *required* versions and extension check here
	unsigned int passedExtCheck = 0x1;
	passedExtCheck &= ( GLEW_VERSION_4_2 );
	passedExtCheck &= ( GLEW_EXT_direct_state_access );

	if( passedExtCheck == false )
		return false;

	glXDestroyContext( (Display*) x11Display, ctx );

	return true;
}

void Gfx::createRenderContexts() {
	const int numThreads = 2; // RENDER_CONTEXT + LOAD_CONTEXT

	int fbcount;
	GLXFBConfig *fbc = glXChooseFBConfig( (Display*) x11Display, DefaultScreen( (Display*) x11Display ), 
													visual_attribs, &fbcount );

	// Pick the FB config/visual with the most samples per pixel
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

	int i;
	for ( i = 0; i < fbcount; i++ )
	{
		XVisualInfo *vi = glXGetVisualFromFBConfig( (Display*) x11Display, fbc[i] );
		if ( vi ) {
			int samp_buf, samples;
			glXGetFBConfigAttrib( (Display*) x11Display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
			glXGetFBConfigAttrib( (Display*) x11Display, fbc[i], GLX_SAMPLES       , &samples  );

			if ( best_fbc < 0 || samp_buf && samples > best_num_samp )
				best_fbc = i, best_num_samp = samples;

			if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
				worst_fbc = i, worst_num_samp = samples;
		}
		XFree( vi );
	}

	GLXFBConfig bestFbc = fbc[ best_fbc ];

	// Be sure to free the FBConfig list allocated by glXChooseFBConfig()
	XFree( fbc );

	GLXContext sharer = 0;
	renderContexts.reset( CORE_NEW_ARRAY RenderContext[ numThreads ] );
	for( int i = 0; i < numThreads; ++i ) {
		GLXContext ctx = 0;
			
		ctx = glXCreateContextAttribsARB( (Display*) x11Display, bestFbc, sharer,
				                          True, context_attribs );
		if( i == 0) {
			sharer = ctx;
		}

//		glXMakeCurrent( (Display*) x11Display, x11Window, ctx );

		renderContexts[i].setGlContext( x11Display, x11Window, ctx );
	}
}

}