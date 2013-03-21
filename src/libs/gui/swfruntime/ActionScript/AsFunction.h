/*
 *  AsFunction.h
 *  SwfPreview
 *
 *  Created by Deano on 20/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASFUNCTION_H_3AA74OHD
#define ASFUNCTION_H_3AA74OHD

#include "AsObject.h"
#include "AsFuncInstruction.h"

namespace Swf {
	class AsAgRuntime;
	class AsVM;
	class AsFunctionBuilder;
	
	class AsFuncBase : public Core::GcBase {
	public:
		virtual AsObjectHandle call( AsAgRuntime* _runtime, AsObjectHandle _this, int _numParams, AsObjectHandle* _params ) const = 0;
		virtual void asReturn() const {};
	};

	class AsNativeFunction : public AsFuncBase {
	public:
		typedef AsObjectHandle (*AsFuncAsAgFunc)( AsAgRuntime*, int, AsObjectHandle* );
			
		AsNativeFunction(AsFuncAsAgFunc _func ) :
			function(_func) {};
				
		AsObjectHandle call( AsAgRuntime* _runtime, AsObjectHandle _this, int _numParams, AsObjectHandle* _params ) const  override {
				return (function)( _runtime, _numParams, _params );
			}
				
	private:
		AsFuncAsAgFunc function;
	};

	template<class T>
	class AsNativeMemberFunction : public AsFuncBase {
	public:
		typedef AsObjectHandle (T::*AsFuncAsObjFunc)( AsAgRuntime*, int, AsObjectHandle* );

		AsNativeMemberFunction(AsFuncAsObjFunc _func ) :
			function( (BaseFunc)_func) {};

		AsObjectHandle call( AsAgRuntime* _runtime, AsObjectHandle _this, int _numParams, AsObjectHandle* _params ) const  override {
			CORE_ASSERT( _this != nullptr );
			return (_this->*function)( _runtime, _numParams, _params );
		}
				
	private:
		typedef AsObjectHandle (AsObject::*BaseFunc)( AsAgRuntime*, int, AsObjectHandle* );
		BaseFunc 					function;
	};

	class AsFunction : public AsFuncBase {
	public:
		AsFunction( const AsFunctionBuilder* _builder );
			
		AsObjectHandle call( AsAgRuntime* _runtime, AsObjectHandle _this, int _numParams, AsObjectHandle* _params ) const  override;
		void asReturn() const override;
	private:
			
		typedef Core::gcvector<AsFuncInstruction*> InstVec;

		InstVec							function;
		mutable bool					returnCalled; // TODO refactor so mutable is needed
	};
} /* Swf */ 

#endif /* end of include guard: ASFUNCTION_H_3AA74OHD */
