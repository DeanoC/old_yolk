/*
 *  AsObjectFunction.h
 *  SwfPreview
 *
 *  Created by Deano on 11/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASOBJECTFUNCTION_H_KCNMNQOQ
#define ASOBJECTFUNCTION_H_KCNMNQOQ

#include "AsObject.h"
#include "AsFunction.h"

namespace Swf {
	class AsAgRuntime;

	class AsObjectFunction : public AsObject {
	public:	
		AsPrimitiveType type() const override {
#if DEBUG
			assert( type ==  APT_FUNCTION );
#endif
			return APT_FUNCTION;
		}		
		AsObjectFunction( const AsFuncBase* _func ) :
	 		AsObject( APT_FUNCTION ),
			value( _func ) {}

		AsObjectFunction( AsObjectHandle (*_func)( AsAgRuntime*, int, AsObjectHandle* ) ) :
			AsObject( APT_FUNCTION ),
			value( CORE_GC_NEW AsNativeFunction(_func) ) {}

		template<class T>
		AsObjectFunction( AsObjectHandle (T::*_func)( AsAgRuntime*, int, AsObjectHandle* ) ) :
			AsObject( APT_FUNCTION ),
			value( CORE_GC_NEW AsNativeMemberFunction<T>(_func) ) {}

	 	const AsFuncBase* value;
	};
} /* Swf */ 

#endif /* end of include guard: ASOBJECTFUNCTION_H_KCNMNQOQ */
