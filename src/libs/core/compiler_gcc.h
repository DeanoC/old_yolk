///-------------------------------------------------------------------------------------------------
/// \file	core\compiler_gcc.h
///
/// \brief	Declares the compiler GCC defines. 
///
/// \details	
/// 	Contains the various defines to support GCC as a compiler for wierd
///
/// \remark	Copyright (c) 2012 Cloud Pixies Ltd. All rights reserved.
/// \remark	mailto://deano@zombiehousestudios.com
/// 
/// \todo GCC force inline needs properly defining
/// \todo properly setup static assert for different GCC versions
////////////////////////////////////////////////////////////////////////////////////////////////////

#define CORE_ALIGN(x)			__attribute__ ((aligned (x)))
#define CORE_STATIC_ASSERT(x)	BOOST_STATIC_ASSERT(x)

#define CORE_INLINE				inline
#define CORE_FORCE_INLINE		inline

#define CALL
#define READONLY_MEM
#define LOCAL_MEM

