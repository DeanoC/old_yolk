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
	namespace AutoGen {	
		class AsAgRuntime;
			
		class AsObjectFactory {
		public:
			typedef AsObjectHandle (*ConstructFunction)( AsAgRuntime*, int, AsObjectHandle* );
			
			AsObjectFactory( AsAgRuntime* _runtime );
						
			void Register( const std::string& _name, ConstructFunction ctor );
			
			AsObjectHandle Construct( const std::string& _name, int _numParams, AsObjectHandle* _params );
		 private:
			typedef Core::gcmap<std::string, ConstructFunction> Constructors;

			Constructors constructors;
			AsAgRuntime* runtime;
		};
	} /* AutoGen */ 
} /* Swf */ 



#endif /* end of include guard: ASOBJECTFACTORY_H_RUH7TJDP */


