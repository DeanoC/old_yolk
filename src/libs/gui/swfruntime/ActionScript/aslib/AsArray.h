/*
 *  AsArray.h
 *  SwfPreview
 *
 *  Created by Deano on 07/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ASARRAY_H_9WZNOYUK
#define ASARRAY_H_9WZNOYUK

#include "../autogen/AsObject.h"
#include "../autogen/AsObjectFactory.h"

namespace Swf
{
	namespace AutoGen
	{

		class AsArray : public AsObject {
		public:
			AsArray() : AsObject( APT_OBJECT ) {}

			virtual void Construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );			
			static AsObjectHandle ConstructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		protected:
		};
		
	} /* AutoGen */ 
	
} /* Swf */ 


#endif /* end of include guard: ASARRAY_H_9WZNOYUK */
