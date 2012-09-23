/*
 *  AsClass.h
 *  SwfPreview
 *
 *  Created by Deano on 25/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ASCLASS_H_XY5W1ODU
#define ASCLASS_H_XY5W1ODU

#include "AsObject.h"
namespace Swf {
	class AsAgRuntime;
		
	class AsClass : public Core::GcBase {
	public:
		typedef AsObjectHandle (AsClass::*Class2FuncPtr)( AsObject* _obj, int _numArgs, AsObjectHandle* _params );
			
		AsClass( AsAgRuntime* _runtime ) : runtime( _runtime ) {}
						
		virtual AsObject* construct( int _numParams, AsObjectHandle* _params ) = 0;

		void registerMemberFunction( const std::string& _name, Class2FuncPtr _func );
		AsObjectHandle callMember( AsObject* _this, const std::string& _name, int _numArgs, AsObjectHandle* _params );
	protected:
		AsAgRuntime* runtime;
		typedef Core::gcmap<std::string, Class2FuncPtr> MemberFunctions;
		MemberFunctions memberFunctions;			
	};
} /* Swf */ 

#endif /* end of include guard: ASCLASS_H_XY5W1ODU */
