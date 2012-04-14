///-------------------------------------------------------------------------------------------------
/// \file	core\compiler_cuda.h
///
/// \brief	Declares the compiler cuda defines. 
///
/// \details	
/// 	Contains the various defines to support CUDA as a compiler for wierd
///
/// \remark	Copyright (c) 2010 Zombie House Studios. All rights reserved.
/// \remark	mailto://deano@zombiehousestudios.com
/// 		
/// \todo does CUDA support alignment attributes for members?
/// \todo can CUDA support CORE_STATIC_ASSERT?
////////////////////////////////////////////////////////////////////////////////////////////////////

#define CORE_ALIGN(x)				(x)
#define CORE_STATIC_ASSERT(x)
#define CORE_INLINE					inline
#define CORE_FORCE_INLINE			inline

#define CALL						__device__
#define READONLY_MEM				__constant__
#define LOCAL_MEM					__device__
