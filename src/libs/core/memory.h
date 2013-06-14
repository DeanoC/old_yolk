/*
 *  memory.h
 *  SwfPreview
 *
 *  Created by Deano on 05/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#ifndef MEMORY_H_ESXYWZO3
#define MEMORY_H_ESXYWZO3

#if defined( USE_TBB )
#	include "tbb/tbbmalloc_proxy.h"
#endif

#if defined( USE_GC )
#	define GC_OPERATOR_NEW_ARRAY
#	define GC_NAME_CONFLICT
#	include "gc_cpp.h"
#endif


namespace Core
{
	extern uint16_t g_MemChkPointId;
#if defined( USE_GC )
	struct GcBase : public gc {};
#else
	struct GcBase {};
#endif
	template<typename T> void GC_Delete(T* tbd ){
		tbd->~T();
		GC_FREE(tbd);
	}
} /* Core */ 

#if defined( USE_GC )
#	define CORE_GC_NEW 				new(UseGC)
#	define CORE_GC_NEW_NOT_ROOT 	new(PointerFreeGC)
#	define CORE_GC_NEW_ROOT_ONLY 	new(NoGC)
#	define CORE_GC_DELETE(x)		Core::GC_Delete(x)
#	define CORE_NEW 				CORE_GC_NEW_NOT_ROOT
#	define CORE_DELETE 				delete
#	define CORE_NEW_ARRAY 			new
#	define CORE_DELETE_ARRAY 		delete[]
#else
#	define CORE_GC_NEW 				new
#	define CORE_GC_NEW_NOT_ROOT 	new
#	define CORE_GC_NEW_ROOT_ONLY 	new
#	define CORE_GC_DELETE(x)		delete x
#	define CORE_NEW 				new
#	define CORE_DELETE 				delete
#	define CORE_NEW_ARRAY 			new
#	define CORE_DELETE_ARRAY 		delete[]

#endif


#if PLATFORM == WINDOWS
#	define CORE_ALIGNED_ALLOC(x,a)	_aligned_malloc(x,a)
#	define CORE_ALIGNED_FREE(x)		_aligned_free(x)
#else
#	define CORE_ALIGNED_ALLOC(x,a)	memalign(x,a)
#	define CORE_ALIGNED_FREE(x)		free(x)
#endif

#define CORE_PLACEMENT_NEW(x)		new(x)

#if PLATFORM == WINDOWS 
#	define CORE_STACK_ALLOC(x)	alloca(x)
#else
#	define CORE_STACK_ALLOC(x)	alloca(x)
#endif

// performa a new onto a stack alloc (rarely useful as type* x = CORE_STACK_NEW(type)(a,b,c); is normally just type x(a,b,c); ) but
// can be useful for dynamic size array
#define CORE_STACK_NEW(type) CORE_PLACEMENT_NEW( CORE_STACK_ALLOC(sizeof(type)) ) type
#define CORE_STACK_NEW_ARRAY(type, count) CORE_PLACEMENT_NEW( CORE_STACK_ALLOC(sizeof(type)*count) ) type


#endif
