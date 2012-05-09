///-------------------------------------------------------------------------------------------------
/// \file	core\core.h
///
/// \brief	Declares the core class. 
///
/// \details	
///		core description goes here
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef WIERD_CORE_CORE_H
#define WIERD_CORE_CORE_H

#if !defined( __STDC_LIMIT_MACROS )
#	define __STDC_LIMIT_MACROS
#endif
#if !defined( __STDC_CONSTANT_MACROS )
#	define __STDC_CONSTANT_MACROS
#endif


// set up the platform level defines
#include "platform.h"

// now include the specific compiler and platform
// set required
#if PLATFORM == WINDOWS
#	if COMPILER == MS_COMPILER
#		include "compiler_ms.h"
#	else
#		error Not supported
#	endif
#	include "platform_win.h"
#elif PLATFORM == GPU
#	if COMPILER == CUDA_COMPILER
#		include "compiler_cuda.h"
#	else
#		error Not supported
#	endif
#elif PLATFORM == APPLE_MAC
#	include "compiler_gcc.h"
#	include "compiler_llvm.h"
#	error Not supported
#elif PLATFORM == IPHONE
//#	include "compiler_gcc.h"
//#	include "compiler_llvm.h"
#	error Not supported
#elif PLATFORM == POSIX
#	if COMPILER == GCC_COMPILER
#		include "compiler_gcc.h"
#	else
#		error Not supported
#	endif
	// TODO detect linux as special case
#	include "platform_posix.h"
#else
#	error Not supported
#endif

// for now whilst performance isn't king allow exceptions
#define USE_CPP_EXCEPTIONS

#define GC_NOT_DLL

//#define USE_GC
#include "memory.h"
#include <cassert>
#include <cmath>
#include <cfloat>
#include <memory>

// standard parts of the engine
#include "core_utils.h"
#if USE_OPENGL
	#if RENDER_BACKEND == Gl 
	#	include "glload\gll.h"
	#	include "glload\gl_4_2.h"
	#	if PLATFORM == WINDOWS
	#	include "glload\wgl_exts.h"
	#	endif
	#endif
#endif

#if PLATFORM != GPU

#	include "debug.h"
#	include "standard.h"				
#	include "exception.h"
#	include "utf8.h"
#	include "singleton.h"

#endif


//!-----------------------------------------------------
//!
//! The core namespace, all core functions not part of
//! separate sub libraries live in here
//!
//!-----------------------------------------------------
namespace Core
{
	void Init( void );		//!< Init core function, call before everything else
	void Shutdown( void );	//!< Shutdown core functions, call last of all
	void HouseKeep( void ); //!< Call every now and again to perform any house keeping
	bool InitWindow( int width, int height, bool bFullscreen ); //!< a simple window opener

	static const int MAX_CMDLINE_ARGS = 1024;	//!< Max cmdline args we support
	extern int g_argc;							//!< global cmdline arg count
	extern char* g_argv[];						//!< global cmdline args

} // namespace Core

	
#endif
