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
#include <sdkddkver.h>
#undef _WIN32_WINNT
#undef _NTDDI_VERSION

#define _WIN32_WINNT _WIN32_WINNT_WINBLUE		
#define _NTDDI_VERSION NTDDI_WINBLUE

#include <windows.h>
#undef ERROR

// C RunTime Header Files
#include <cstdlib>
#include <tchar.h>
#include <cassert>