/** \file debug.cpp
   debug C++ file.
   Provides a logger class
   (c) Dean Calver
 */
//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "core/core.h"

#if PLATFORM_OS == OSX && defined(__APPLE_API_UNSTABLE)
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <uniCore.h>
#include <sys/sysctl.h>
#endif
#include "debug.h"

#if defined( USE_GLOG )
namespace Core {
   const std::string Logger::endl = "\n";
}
#endif

//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Local Classes and structures
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Local Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
uint16_t Core::g_MemChkPointId = 0;


#if !defined( USE_GLOG )

Core::Logger			Log;

#if !defined(NDEBUG) && PLATFORM == WINDOWS
namespace
{
	HANDLE g_WarnFile;
	HANDLE g_ErrFile;
	HANDLE g_AssertFile;
}
#endif

namespace Core
{

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

void Logger::Init( void )
{
#if !defined(NDEBUG) && PLATFORM == WINDOWS
	// Get the current state of the flag
	// and store it in a temporary variable
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn On (OR) - Keep freed memory blocks in the
	// heapís linked list and mark them as freed
	tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;

	// Turn On (OR) - On exit report leaks
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

	// Set the new state for the flag
	_CrtSetDbgFlag( tmpFlag );

	TCHAR	modName[255];
	GetModuleFileName( NULL, modName, 255);
	int curChar = 0;

	while( modName[curChar] != 0 )
	{
		if( modName[curChar] == '.' )
		{
			modName[curChar] = 0;
			break;
		}
		++curChar;
	}

	std::string iname = std::string(modName) + "_info.log";
	g_WarnFile = CreateFile( iname.c_str(),			// filename
							GENERIC_WRITE,	// write only
							FILE_SHARE_WRITE,	// no sharing
							0,				// default security
							OPEN_ALWAYS,	// new file everytime
							FILE_ATTRIBUTE_NORMAL | // normal file 
							FILE_FLAG_WRITE_THROUGH, // no write cache
							0 );				// no template
	std::string ename = std::string(modName) + "_error.log";
	g_ErrFile = CreateFile(	ename.c_str(),			// filename
							GENERIC_WRITE,	// write only
							0,				// no sharing
							0,				// default security
							CREATE_ALWAYS,	// new file everytime
							FILE_ATTRIBUTE_NORMAL | // normal file 
							FILE_FLAG_WRITE_THROUGH, // no write cache
							0 );				// no template
	std::string wname = std::string(modName) + "_assert.log";
	g_AssertFile = CreateFile(	wname.c_str(),			// filename
							GENERIC_WRITE,	// write only
							0,				// no sharing
							0,				// default security
							CREATE_ALWAYS,	// new file everytime
							FILE_ATTRIBUTE_NORMAL | // normal file 
							FILE_FLAG_WRITE_THROUGH, // no write cache
							0 );				// no template

	_CrtSetReportFile( _CRT_WARN, g_WarnFile );
	_CrtSetReportFile( _CRT_ERROR, g_WarnFile );
	_CrtSetReportFile( _CRT_ASSERT, g_WarnFile );
	_CrtSetReportFile( _CRT_ERROR, g_ErrFile );
	_CrtSetReportFile( _CRT_ASSERT, g_AssertFile );

	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW );

#endif
}

void Logger::Shutdown( void )
{
#if !defined(NDEBUG) && PLATFORM == WINDOWS
	CloseHandle( g_WarnFile );
	CloseHandle( g_ErrFile );
	CloseHandle( g_AssertFile );
#endif
}

} // namespace Core

#endif // end !HAS_GLOG

namespace Core {

void SetBreakOnAlloc( int id )
{
#if !defined(NDEBUG) && PLATFORM == WINDOWS
	_CrtSetBreakAlloc( id );
#endif
}


bool InDebugger() {
#if PLATFORM_OS == OSX && defined(__APPLE_API_UNSTABLE)
	int                 junk;
	int                 mib[4];
	kinfo_proc   info;
	size_t              size;
	
	// Initialize the flags so that, if sysctl fails for some bizarre
	// reason, we get a predictable result.	
	info.kp_proc.p_flag = 0;
	
	// Initialize mib, which tells sysctl the info we want, in this case
	// we're looking for information about a specific process ID.
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = getpid();
	
	// Call sysctl.
	size = sizeof(info);
	junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
	CORE_ASSERT(junk == 0);
	// We're being debugged if the P_TRACED flag is set.
	return ( (info.kp_proc.p_flag & P_TRACED) != 0 );	
#elif  PLATFORM_OS == MS_WINDOWS
	return IsDebuggerPresent() != 0;
#else
	return false;
#endif
}

#if PLATFORM_OS == MS_WINDOWS
void TakeMemoryCheckPoint( MemoryCheckPoint& chkpoint )
{
	_CrtMemCheckpoint( &chkpoint.state );
	chkpoint.id = g_MemChkPointId++;
}
namespace
{
	class MemoryChkPntHelper
	{
	public:
		// brain tells me there must be a key only container in std c++ but 
		// buggered if I can think/find it at the mo..
		typedef Core::unordered_map< void*, void* > KeyArray;
		KeyArray leakedPtrs;
		
		static void CrtClientDumper( void* data, void* context )
		{
			uint32_t ids = (*((uint32_t*)context));
			uint16_t oldid = (ids & 0xFFFF);
			uint16_t newid = (ids >> 16);
			int block = _CrtReportBlockType( data );
			uint16_t subtype = (uint16_t) _BLOCK_SUBTYPE(block);
			if( g_MemoryChkPntHelper != data && 
				subtype > oldid &&
				subtype <= newid 
				)
			{
				g_MemoryChkPntHelper->leakedPtrs[ data ] = data;
			}
		}

		static void CrtAllDumper( void* data, size_t size )
		{
			// check its a leaked one
			if( g_MemoryChkPntHelper->leakedPtrs.find( data ) !=  
								g_MemoryChkPntHelper->leakedPtrs.end() )
			{
				long memNum;
				char* filename;
				int lineNum;

				if( _CrtIsMemoryBlock( data, size, &memNum, &filename, &lineNum ) )
				{
					if( filename != NULL ) 
					{
						LOG_FILE_LINE( INFO, filename, lineNum )
							<< "Mem Leak @ 0x" << data << " size " << size << " bytes\n";
					} else
					{
						LOG(INFO) << "Mem Leak @ 0x" << data << " size " << size << " bytes\n";
					}
				}
			}
		}
	} *g_MemoryChkPntHelper;
}
bool CompareMemoryCheckPoints( const MemoryCheckPoint& oldc, const MemoryCheckPoint& newc )
{
	uint32_t ids = oldc.id | (((uint32_t)newc.id) << 16);
	g_MemoryChkPntHelper = CORE_NEW MemoryChkPntHelper;
	_CrtDoForAllClientObjects( &g_MemoryChkPntHelper->CrtClientDumper, &ids );
	if( g_MemoryChkPntHelper->leakedPtrs.empty() == true )
	{
		CORE_DELETE( g_MemoryChkPntHelper );
		return true;
	}

	MemoryCheckPoint diff;
	if ( _CrtMemDifference( &diff.state, &oldc.state, &newc.state ) )
	{
		_CrtMemDumpStatistics( &diff.state );
		_CrtSetDumpClient( &g_MemoryChkPntHelper->CrtAllDumper );
		_CrtMemDumpAllObjectsSince( &oldc.state );
		_CrtSetDumpClient( NULL );
	}
	CORE_DELETE( g_MemoryChkPntHelper );
	return false;
}
#else

void TakeMemoryCheckPoint( const MemoryCheckPoint& chkpoint ) 
{
	LOG(INFO) << "Memory Checkpoints not implemented on this platform\n";
}
bool CompareMemoryCheckPoints( const MemoryCheckPoint& oldc, const MemoryCheckPoint& newc )
{
	return false;
}

#endif


} // end namespace Core
//---------------------------------------------------------------------------
//  FUNCTION NAME	: 
//	PARAMETERS		:
//	RETURN			: 
//	EXCEPTIONS		:
//	DESCRIPTION		:
//	NOTES			:
//---------------------------------------------------------------------------
