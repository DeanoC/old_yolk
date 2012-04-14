/** \file debug.h
 * Debug Header.
 * provides macros to enable easy debugging! (I wish)
 * (c) 2000 Dean Calver
 */

#pragma once

#ifndef WIERD_CORE_DEBUG_H
#define WIERD_CORE_DEBUG_H

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

/// Remove all debugging/assert check
#define DEBUG_NONE	0
/// Minimal debugging/runtime assertation help
#define DEBUG_LOW	1
/// Average debugging/runtime assertation help
#define DEBUG_MED	2
/// Maximum (assert everything) debugging checks
#define DEBUG_HIGH	3

#if DEBUG_LEVEL >= DEBUG_NONE
	/// Do this on debug builds only
#	define IF_DEBUG(x) x
	/// start a section of code for debugging only
#	define IF_DEBUG_START if(true){
	/// end a section of code for debugging only
#	define IF_DEBUG_END	 }
#else
	/// Do this on debug builds only, so remove completely
#	define IF_DEBUG(x)
	/// dead code optimiser will remove the code below
#	define IF_DEBUG_START if(false){
#	define IF_DEBUG_END	 }
#endif

#if DEBUG_LEVEL >= DEBUG_LOW
	/// Do this on debug builds only
#	define IF_DEBUG_LOW(x) x
#else
	/// Do this on debug builds only, so remove completely
#	define IF_DEBUG_LOW(x)
#endif

#if DEBUG_LEVEL >= DEBUG_MED
	/// Do this on debug builds only
#	define IF_DEBUG_MED(x) x
#else
	/// Do this on debug builds only, so remove completely
#	define IF_DEBUG_MED(x)
#endif

#if DEBUG_LEVEL >= DEBUG_HIGH
	/// Do this on debug builds only
#	define IF_DEBUG_HIGH(x) x
#else
	/// Do this on debug builds only, so remove completely
#	define IF_DEBUG_HIGH(x)
#endif

/// Assert and throw a incorrect parameter exception if assert fails
#define PARAM_ASSERT( x ) if( !(x) ){ CoreThrowException(ParamError, "(" #x ")" " == false"); }
/// Assert and throw a TODO exception if assert fails
#define TODO_ASSERT( x ) if( !(x) ){ CoreThrowException(TodoError, "(" #x ")" " == false"); }
/// Assert and throw a TODO exception if assert fails
#define CORE_ASSERT( x ) if( !(x) ){ CoreThrowException(AssertError, "(" #x ")" " == false"); }

/// assert that memory pointer is not NULL
#define MEM_CHK( x) if( (x) == 0 ){ CoreThrowException(MemError, #x); }

#define SAFE_RELEASE(x) if( (x) ){ (x)->Release(); (x) = 0; }

#if defined( USE_GLOG )
#	include "glog/logging.h"
#else
// glog compat layer (not not all glog features support but should compile and log somthing...)
// ignore GLOG level and just log something...

#define LOG(c) Log 

	// older log library left in for now...
	// Include OS version specific debug function
#	if(PLATFORM == WINDOWS)
#		include "platform_windows/debug_win.h"
#	else
#		include "platform_posix/debug_Core.h"
#	endif

	namespace Core {

	/**
	Logger class.
	Has a templated member which passes any << to system debugger
	Log << "Log the number five now ----" << 5;
	*/
	class Logger
	{
	public:
		enum eLNL { endl };
		enum eInfo { info };
		enum eError { error };

		Logger() : m_bInfo( true ) {}

		/// sets up the logs etc
		static void Init( void );
		///! close the logs etc
		static void Shutdown( void );

		/// templated operator calls OS specfic templated log class
		template<class T> const Logger& operator<<(const T& in) const {
			if( m_bInfo )
				DebugInfo(in);
			else
				DebugError(in);

			return *this;
		}

		/// adds a new line (same as <<"\n")
		const Logger& operator<<(eLNL) const	{ DebugLogNL();	m_bInfo = true; return *this; };
		/// Changes output to the warning stream
		const Logger& operator<<(eInfo) const	{ m_bInfo = true; return *this; };
		/// Changes output to the error stream
		const Logger& operator<<(eError) const	{ m_bInfo = false; return *this; };

	private:
		//! are we in the info or error state
		mutable bool m_bInfo;
	};

	} // namespace Core

	extern Core::Logger Log; // The log is left in the global namespace for easy access

#endif

	/// Log file and line macros in a VS compat manner
#	define LOG_FILE_LINE( lev, f, l ) LOG(lev) << (f) << '(' << (l) << ')' << " : "
	/// Log file and line macros for a code file and line
#	define LOGFL LOG_FILE_LINE( lev, __FILE__, __LINE__ )

namespace Core 
{
	///-------------------------------------------------------------------------------------------------
	/// \typedef	void* MemoryCheckPoint
	///
	/// \brief	Defines an alias representing the memory check point.
	////////////////////////////////////////////////////////////////////////////////////////////////////
#if PLATFORM_OS == MS_WINDOWS
	typedef struct { _CrtMemState state; uint16_t id; } MemoryCheckPoint;
#else
	typedef void* MemoryCheckPoint;
#endif

	///-------------------------------------------------------------------------------------------------
	/// \fn	void SetBreakOnAlloc( int id )
	///
	/// \brief	Sets a break on memory allocation. 
	///
	/// \param	id	The identifier of the allocation. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetBreakOnAlloc( int id ); 

	///-------------------------------------------------------------------------------------------------
	/// \fn	bool InDebugger()
	///
	/// \brief	Returns if we are currenting running the debugger. 
	///
	/// \return	true if in debugger , false if not. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool InDebugger();

	///---------------------------------------------------------------------------------------------
	/// \fn	void TakeMemoryCheckPoint( const MemoryCheckPoint& chkpoint )
	///
	/// \brief	Take memory check point. 
	///
	/// \param	chkpoint	The chkpoint. 
	////////////////////////////////////////////////////////////////////////////////////////////////
	void TakeMemoryCheckPoint( MemoryCheckPoint& chkpoint );

	///---------------------------------------------------------------------------------------------
	/// \fn	bool CompareMemoryCheckPoints( const MemoryCheckPoint& old,const MemoryCheckPoint& new )
	///
	/// \brief	Compare memory check points. Any differences dumped to the info log.
	///
	/// \param	old	The old. 
	/// \param	new	The new. 
	///
	/// \return	true if they are the same, false if different. 
	////////////////////////////////////////////////////////////////////////////////////////////////
	bool CompareMemoryCheckPoints( const MemoryCheckPoint& oldc, const MemoryCheckPoint& newc );
} //namespace Core


#if (PLATFORM_OS == OSX)
#	if defined(DEBUG)
#		if __ppc64__ || __ppc__
#			define DebugBreak() if(Core::InDebugger()) { \
					__asm__("li r0, 20\nsc\nnop\nli r0, 37\nli r4, 2\nsc\nnop\n" \
								: : : "memory","r0","r3","r4" ); }
#		else
#			define DebugBreak() if(Core::InDebugger()) {__asm__("int $3\n" : : );}
#		endif
#	else
#		define DebugBreak()
#	endif
#endif

#endif
