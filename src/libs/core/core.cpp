///-------------------------------------------------------------------------------------------------
/// \file	core\core.cpp
///
/// \brief	Implements the core library itself (stuff to control core). 
///
/// \details	
///		core description goes here
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "core.h"
#include "resourceman.h"
#include "coreresources.h"
#include "development_context.h"
#include "clock.h"
#include "sysmsg.h"
#include "keyboard.h"

#if PLATFORM == WINDOWS
#include "platform_windows/mouse_win.h"

extern void WinGetMessages( void );
extern bool WinInitWindow( int width, int height, bool bFullscreen );
bool InWinCrtReportLog = false;
int __cdecl WinCrtReportHook(int type, char * msg, int * ret ) {
   if(InWinCrtReportLog)
      return FALSE;

	switch( type ) {
	case _CRT_WARN:		LOG(INFO) << msg; break;
	case _CRT_ERROR:	LOG(ERROR) << msg; break;
	case _CRT_ASSERT:	LOG(FATAL) << msg; break;
	}
	return FALSE;
}
#endif

#if defined(USE_GC)
#include "private/gc_priv.h"
#endif

namespace Core 
{

int g_argc = 0;						//!< The argc cmdline parameter
char* g_argv[ MAX_CMDLINE_ARGS ];	//!< The argv cmdline parameter

//!-----------------------------------------------------
//!
//! Initialisation of the core library
//!
//!-----------------------------------------------------


///-------------------------------------------------------------------------------------------------
/// \fn	void Init( void )
///
/// \brief	Initialisation of the core library. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
void Init( void ) {
	Clock::init();
	SystemMessage::init();
	ResourceMan::init();
	InstallResourceTypes();
#if PLATFORM == WINDOWS 
//	_CrtSetDbgFlag( _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_ALLOC_MEM_DF);
//	_CrtSetReportHook( &WinCrtReportHook );
#endif

#if defined( USE_GLOG )
	FLAGS_log_dir = ".";
	FLAGS_alsologtostderr = true;
#else
	Logger::init();
#endif 
	DevelopmentContext::init();
#if defined(USE_GC)
	GC_enable_incremental();
	GC_init();
#endif
}

///-------------------------------------------------------------------------------------------------
/// \fn	void Shutdown( void )
///
/// \brief	Shutdowns the core library. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
void Shutdown( void ) {
	DevelopmentContext::shutdown();
#if !defined( USE_GLOG )
	Logger::shutdown();
#endif
	ResourceMan::shutdown();
	SystemMessage::shutdown();
	Clock::shutdown();
	Keyboard::shutdown();
#if PLATFORM == WINDOWS 
	MouseWin::shutdown();

	_CrtSetReportHook( NULL );
	_CrtSetDbgFlag( 0 );
#endif
}

///-------------------------------------------------------------------------------------------------
/// \fn	void HouseKeep( void )
///
/// \brief	Performs some background house keeping. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
void HouseKeep( void ) {
#if defined(USE_GC)
//	GC_gcollect();
#endif

#if PLATFORM == WINDOWS
	WinGetMessages();
#elif PLATFORM == POSIX
	extern void X11PumpEvents();

	X11PumpEvents();
#endif
}

///-------------------------------------------------------------------------------------------------
/// \fn	bool InitWindow( int width, int height, bool bFullscreen )
///
/// \brief	If on windowable OS opens a window, else does nothing. 
///
/// \param	width		The width. 
/// \param	height		The height. 
/// \param	bFullscreen	true to fullscreen. 
///
/// \return	true if it succeeds (or if not on a windowed system), false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool InitWindow( int width, int height, bool bFullscreen ) {
#if PLATFORM == WINDOWS
	bool ret = WinInitWindow( width, height, bFullscreen );
	if( ret ) {
		Keyboard::init();
		MouseWin::init();
	}

	return ret;
#else
	Keyboard::init();
	return true;
#endif
}

} // end namespace
