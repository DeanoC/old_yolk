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

#include "../AsObject.h"
#include "../AsObjectFactory.h"

namespace Swf
{
	class AsArray : public AsObject {
	public:
		AsArray() : AsObject( APT_OBJECT ) {}

		virtual void construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );			
		static AsObjectHandle constructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
	protected:
	};
} /* Swf */ 


#endif /* end of include guard: ASARRAY_H_9WZNOYUK */
