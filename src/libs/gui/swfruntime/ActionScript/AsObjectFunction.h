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
		virtual AsPrimitiveType type() {
#if DEBUG
			assert( type ==  APT_FUNCTION );
#endif
			return APT_FUNCTION;
		}		
		AsObjectFunction( const Swf::AsFuncBase* _func ) :
	 		AsObject( APT_FUNCTION ),
			value( _func ) {}

		AsObjectFunction( AsObjectHandle (AsAgRuntime::*_func)( int, AsObjectHandle* ) ) :
			AsObject( APT_FUNCTION ),
			value( CORE_NEW Swf::AsAgFunction(_func) ) 
		{	
		}

	 	const Swf::AsFuncBase* value;
	};

	class AsObjectThisFunction : public AsObject {
	public:	
		virtual AsPrimitiveType type() {
#if DEBUG
			assert( type ==  APT_FUNCTION );
#endif
			return APT_FUNCTION;
		}		
		AsObjectThisFunction( const Swf::AsFuncThisBase* _func ) :
 			AsObject( APT_FUNCTION ),
			value( _func ) {}
		
		template<class T>
		AsObjectThisFunction( AsObjectHandle (T::*_func)( AsAgRuntime*, int, AsObjectHandle* ) ) :
			AsObject( APT_FUNCTION ),
			value( CORE_NEW Swf::AsObjFunction<T>(_func) ) 
		{	
		}
	
 		const Swf::AsFuncThisBase* value;
	};	
} /* Swf */ 

#endif /* end of include guard: ASOBJECTFUNCTION_H_KCNMNQOQ */