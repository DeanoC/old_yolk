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

#define CORE_ALIGN(x)			__declspec(align(x))

#define CORE_INLINE				inline
#define CORE_FORCE_INLINE		__forceinline 

#define CALL
#define READONLY_MEM
#define LOCAL_MEM

// Boost needs building with the EXACT same settings, commands below (x86 and x64)
// b2 -j5 -a --build-type=complete --without-mpi define=_SECURE_SCL=0 define=_HAS_ITERATOR_DEBUGGING=0 define=_CRT_NONSTDC_NO_DEPRECATE define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECATE
// b2 -j5 -a --build-type=complete --without-mpi define=_SECURE_SCL=0 define=_HAS_ITERATOR_DEBUGGING=0 define=_CRT_NONSTDC_NO_DEPRECATE define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECATE msvc architecture=x86 address-model=64 --stagedir=./stage64
#define _HAS_ITERATOR_DEBUGGING			0
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#define _SECURE_SCL						0 

#define __TBB_NO_IMPLICIT_LINKAGE		1