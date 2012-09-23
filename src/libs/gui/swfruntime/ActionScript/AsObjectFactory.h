/*
 *  AsObjectFactory.h
 *  SwfPreview
 *
 *  Created by Deano on 25/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef ASOBJECTFACTORY_H_RUH7TJDP
#define ASOBJECTFACTORY_H_RUH7TJDP

#include "AsObject.h"

namespace Swf {
	class AsAgRuntime;
			
	class AsObjectFactory {
	public:
		typedef AsObjectHandle (*ConstructFunction)( AsAgRuntime*, int, AsObjectHandle* );
			
		AsObjectFactory( AsAgRuntime* _runtime );
						
		void registerFunc( const std::string& _name, ConstructFunction ctor );
			
		AsObjectHandle construct( const std::string& _name, int _numParams, AsObjectHandle* _params );
		private:
		typedef Core::gcmap<std::string, ConstructFunction> Constructors;

		Constructors constructors;
		AsAgRuntime* runtime;
	};
} /* Swf */ 



#endif /* end of include guard: ASOBJECTFACTORY_H_RUH7TJDP */


