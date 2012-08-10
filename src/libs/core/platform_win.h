#	if COMPILER_VERSION < MS_VS2010
#	include "pstdint.h"	
#	else
#	include <stdint.h>
#endif
//#	define _CRTDBG_MAP_ALLOC
//#	include <crtdbg.h>

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#undef NOMINMAX
#define NOMINMAX
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500		// Windows XP or greater
#include <windows.h>
#undef ERROR

// C RunTime Header Files
#include <cstdlib>
#include <tchar.h>
#include <cassert>

#if defined( USE_DX11 )
#	define USE_DINPUT
#endif