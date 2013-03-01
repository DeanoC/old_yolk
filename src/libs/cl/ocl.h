#pragma once

#if !defined(YOLK_CL_STDAFX_H)
#define YOLK_CL_STDAFX_H

#include "core/core.h"
#include "core/vector_math.h"

// cl + all extension (includign GL and DX interop)
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/opencl.h>
typedef CL_API_ENTRY cl_event (CL_API_CALL *clCreateEventFromGLsyncKHR_fn)(cl_context           /* context */,
                           cl_GLsync            /* cl_GLsync */,
                           cl_int *             /* errcode_ret */);

// Rename references to this dynamically linked function to avoid
// collision with static link version
#define clGetGLContextInfoKHR clGetGLContextInfoKHR_proc
#define clCreateEventFromGLsyncKHR clCreateEventFromGLsyncKHR_proc
extern clGetGLContextInfoKHR_fn clGetGLContextInfoKHR;
extern clCreateEventFromGLsyncKHR_fn clCreateEventFromGLsyncKHR;
extern void clPrintErr(cl_int errcode, const char *file, const int line, const char *msg);

#define CL_CHECK( expr, _err)															\
	do {																				\
		if (_err == CL_SUCCESS) break;													\
		::clPrintErr(_err, __FILE__, __LINE__, expr );											\
	} while (0)

namespace Cl {
	enum DEVICE_EXTENSIONS_FLAGS {
		DEF_IMAGES							= BIT(0),
		DEF_FULL_PROFILE					= BIT(1),
		DEF_AMD_PRINTF						= BIT(2),
		DEF_KHR_GLOBAL_EXT_ATOMIC_I32		= BIT(3),
		DEF_KHR_LOCAL_EXT_ATOMIC_I32		= BIT(4),
		DEF_KHR_3D_IMAGE_WRITES				= BIT(5),
		DEF_EXT_ATOMIC_COUNTERS_32			= BIT(6),
		DEF_AMD_VEC3						= BIT(7),
		DEF_AMD_MEDIA_OPS					= BIT(8),
		DEF_AMD_POPCNT						= BIT(9),
		DEF_EXT_DEVICE_FISSION				= BIT(10),

		DEF_NONE = 0,
	};
}

#endif // WIERD_CL_STDAFX_H
