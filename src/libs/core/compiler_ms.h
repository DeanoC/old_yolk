///-------------------------------------------------------------------------------------------------
/// \file	core\compiler_ms.h
///
/// \brief	Declares the compiler milliseconds class. 
///
/// \details	
///		compiler_ms description goes here
///
/// \remark	Copyright (c) 2010 Cloud Pixies Ltd. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
/// \todo	Fill in detailed file description. 
/// \todo	support iterator debugging in debug
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define DEBUG_LEVEL DEBUG_MED
#endif

#define CORE_ALIGN(x)			__declspec(align(x))

#define CORE_INLINE				inline
#define CORE_FORCE_INLINE		__forceinline 

#define CALL
#define READONLY_MEM
#define LOCAL_MEM

#define AMP_ONLY			restrict(amp)
#define AMP_ALLOWED			restrict(cpu,amp)

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

#define __TBB_NO_IMPLICIT_LINKAGE		1

