///-------------------------------------------------------------------------------------------------
/// \file	core\compiler_llvm.h
///
/// \brief	Declares the compiler llvm class. 
///
/// \details	
///		compiler_llvm description goes here
///
/// \remark	Copyright (c) 2012 Cloud Pixies Ltd. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
/// \todo	Test and support LLVM
////////////////////////////////////////////////////////////////////////////////////////////////////

#define CORE_ALIGN(x)				__attribute__ ((aligned (x)))
#define CORE_STATIC_ASSERT(x)	BOOST_STATIC_ASSERT(x)

#define CORE_INLINE					inline
#define CORE_FORCE_INLINE			inline

#define CALL
#define READONLY_MEM
#define LOCAL_MEM

