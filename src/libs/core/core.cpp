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
#include "core.h"

#if PLATFORM == WINDOWS
#include "platform_windows/mouse_win.h"
#include "platform_windows/keyboard_win.h"

extern void WinGetMessages( void );
extern bool WinInitWindow( int width, int height, bool bFullscreen );
bool InWinCrtReportLog = false;
int __cdecl WinCrtReportHook(int type, char * msg, int * ret )
{
   if(InWinCrtReportLog)
      return FALSE;

	switch( type )
	{
	case _CRT_WARN:		LOG(INFO) << msg; break;
	case _CRT_ERROR:	LOG(ERROR) << msg; break;
	case _CRT_ASSERT:	LOG(ERROR) << msg; break;
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
void Init( void ) 
{
	Clock::Init();
	SystemMessage::Init();
	ResourceMan::Init();
	InstallResourceTypes();
#if PLATFORM == WINDOWS 
	_CrtSetDbgFlag( _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_ALLOC_MEM_DF);
	_CrtSetReportHook( &WinCrtReportHook );
#endif

#if defined( USE_GLOG )
	FLAGS_log_dir = ".";
	FLAGS_alsologtostderr = true;
#else
	Logger::Init();
#endif 
	DevelopmentContext::Init();
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
void Shutdown( void ) 
{
	DevelopmentContext::Shutdown();
#if !defined( USE_GLOG )
	Logger::Shutdown();
#endif
	ResourceMan::Shutdown();
	SystemMessage::Shutdown();
	Clock::Shutdown();
#if PLATFORM == WINDOWS 
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
void HouseKeep( void ) 
{
#if defined(USE_GC)
	GC_gcollect();
#endif

#if PLATFORM == WINDOWS
	if( KeyboardWin::Exists() ) {
		KeyboardWin::Get()->update();
	}

	if( MouseWin::Exists() ) {
		Core::MouseWin::Get()->update();
	}

	WinGetMessages();
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
		KeyboardWin::Init();
//		MouseWin::Init();
	}

	return ret;
#else
	return true;
#endif
}

} // end namespace

#if defined(USE_GC)
#include "ThirdParty/gc/gc_cpp.cc"
#endif