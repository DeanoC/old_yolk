/*
 *  AsDate.h
 *  SwfPreview
 *
 *  Created by Deano on 27/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ASDATE_H_QGTI5HXD
#define ASDATE_H_QGTI5HXD

#include "../AsObject.h"
#include "../AsObjectFactory.h"
namespace Swf
{
	class AsDate : public AsObject {
	public:
		AsDate() : AsObject( APT_OBJECT ) {}

		virtual void construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );			
		static AsObjectHandle constructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
	
	protected:
		AsObjectHandle getHours( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params );
		AsObjectHandle getMinutes( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params );
		AsObjectHandle getSeconds( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params );
		AsObjectHandle getDay( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		AsObjectHandle getDate( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		AsObjectHandle getMonth( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
		AsObjectHandle getFullYear( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params );
	};
	
} /* Swf */ 


#endif /* end of include guard: ASDATE_H_QGTI5HXD */
