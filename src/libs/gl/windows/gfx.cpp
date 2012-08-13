#include "ogl.h"
#include "wglew.h"
#include "gfx.h"
#include "rendercontext.h"
#include "core/resourceman.h"
#include "core/platform_windows/win_shell.h"

//#define GDEBBUGGER_FRIENDLY_STARTUP

#pragma comment( lib, "opengl32" )
namespace Gl {

extern void debugOutputAMD(GLuint id, GLenum category, GLenum severity, GLsizei, const GLchar* message, GLvoid*);
extern void debugOutput ( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam );

#if defined(GDEBBUGGER_FRIENDLY_STARTUP)

HGLRC initialGLRC;
bool Gfx::createGLContext() {
	HDC hDC=GetDC(g_hWnd);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR); 
    pfd.nVersion        = 1; 
    pfd.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER ;
    pfd.iPixelType      = PFD_TYPE_RGBA; 
    pfd.cColorBits      = 24; 
    pfd.cRedBits        = 8; 
    pfd.cGreenBits      = 8; 
    pfd.cBlueBits       = 8; 
    pfd.cAlphaBits      = 8;
    pfd.cDepthBits      = 24; 
    pfd.cStencilBits    = 8; 
    pfd.iLayerType      = PFD_MAIN_PLANE; 

	int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (nPixelFormat == 0) return false;
 
	BOOL bResult = SetPixelFormat (hDC, nPixelFormat, &pfd); 
	if (!bResult) return false; 
	HGLRC tempContext = wglCreateContext(hDC);
	wglMakeCurrent(hDC, tempContext);

	GLenum err = glewInit();
	if (GLEW_OK != err)
		return  false;

	// *required* versions and extension check here
	unsigned int passedExtCheck = 0x1;
	passedExtCheck &= ( GLEW_VERSION_4_2 );
//	passedExtCheck &= ( GLEW_EXT_direct_state_access );

	if( passedExtCheck == false )
		return false;	
 
//	wglMakeCurrent(NULL,NULL);

	// done with dummy now!
//	wglDeleteContext(tempContext);

	initialGLRC = tempContext;
	return true;
}

#else
HGLRC tempContext;
HWND temphWnd;
bool Gfx::createGLContext() {

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "WierdDummy";
	wcex.hIconSm = NULL;
	if( !RegisterClassEx( &wcex ) )
		return false;

	// create a dummy window, so we can have a dummy gl context, to get the real useful context
	// attached to the real useful window... and D3D is hard to setup!
	HWND hWnd=CreateWindow(	_T("WierdDummy"), 
						_T(""), 
						WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, 
						0,0,0,0, 0, 0, 0, 0 );
	HDC hDC=GetDC(hWnd);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
 
	int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (nPixelFormat == 0) return false;
 
	BOOL bResult = SetPixelFormat (hDC, nPixelFormat, &pfd); 
	if (!bResult) return false; 

	tempContext = wglCreateContext(hDC);
	wglMakeCurrent(hDC, tempContext);

	GLenum err = glewInit();
	if (GLEW_OK != err)
		return  false;

	// *required* versions and extension check here
	unsigned int passedExtCheck = 0x1;
	passedExtCheck &= ( GLEW_VERSION_4_2 );
//	passedExtCheck &= ( GLEW_EXT_direct_state_access );

	if( passedExtCheck == false )
		return false;	

	return true;
}

bool Gfx::setGlPixelFormat(uint32_t flags ) {
	// for out output buffer, we simple want a double buffered colour, nothing else
	// all rendering will be done to off screen buffers, then blited to here for display
	int pixelFormat;
	UINT numFormats;
	HDC hDC = GetDC(g_hWnd);
 
	int iAttributes[] = { 
		WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
		WGL_ACCELERATION_ARB,		WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,			24,
		WGL_ALPHA_BITS_ARB,			8,
		WGL_DEPTH_BITS_ARB,			0,
		WGL_STENCIL_BITS_ARB,		0,
		WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
		WGL_STEREO_ARB,				(flags & Scene::SCRF_STEREO) ? GL_TRUE : GL_FALSE,
		0, 0 };

	if( wglChoosePixelFormatARB( hDC, iAttributes, NULL, 1, 
									&pixelFormat, &numFormats ) == false )
		return false;

	if( SetPixelFormat( hDC, pixelFormat, NULL ) == false ) 
		return false;	

	wglMakeCurrent( hDC, NULL );

	// done with dummy now!
	wglDeleteContext(tempContext);
	DestroyWindow( temphWnd );
 
	return true;	
}

#endif
void Gfx::createRenderContexts() {
	HDC hDC=GetDC(g_hWnd);
	int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
		IF_DEBUG(
			| WGL_CONTEXT_DEBUG_BIT_ARB
		)
		, 0, 0
	};

	const std::string testStr( "Gl debugging callback installed" );
 
	renderContexts.push_back( std::unique_ptr<RenderContext>( CORE_NEW RenderContext() ) );
	renderContexts.push_back( std::unique_ptr<RenderContext>( CORE_NEW RenderContext() ) );

	for( int i = 0; i < renderContexts.size(); ++i ) {
		HGLRC hRC = 
#if defined(GDEBBUGGER_FRIENDLY_STARTUP)
			( i == 0 ) ? initialGLRC : wglCreateContext(hDC);
#else
			wglCreateContextAttribsARB(hDC,0, attribs);
#endif
		auto rc = (Gl::RenderContext*)renderContexts[i].get();
		rc->setGlContext( hDC, hRC );
		if( i != 0 ) {
			wglShareLists( ((Gl::RenderContext*)renderContexts[0].get())->hRC, rc->hRC );
		}
		wglMakeCurrent( hDC,hRC );
		GL_CHECK
#if !defined(GDEBBUGGER_FRIENDLY_STARTUP)
		if( i == 0 ) {
			if( glDebugMessageCallbackAMD ) {
				glDebugMessageCallbackAMD(&debugOutputAMD, NULL);
				GL_CHECK
				glDebugMessageEnableAMD(0, 0, 0, NULL, GL_TRUE);
				GL_CHECK
				glDebugMessageInsertAMD(GL_DEBUG_CATEGORY_APPLICATION_AMD, 
					GL_DEBUG_SEVERITY_LOW_AMD, 1, testStr.length(), testStr.c_str() );
				GL_CHECK
			} else if(glDebugMessageControlARB) {
				glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
				GL_CHECK
				glDebugMessageCallbackARB(&debugOutput, NULL);
				GL_CHECK
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
				GL_CHECK
				glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 
					1, GL_DEBUG_SEVERITY_LOW_ARB, testStr.length(), testStr.c_str() );
				GL_CHECK
			}
		}
#endif
	}

	renderContexts[0]->threadActivate();
}

} // end namespace