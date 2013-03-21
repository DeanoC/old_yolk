/*
 *  AsArray.cpp
 *  SwfPreview
 *
 *  Created by Deano on 07/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include <sstream>
#include "../AsObjectFactory.h"
#include "../AsAgRuntime.h"
#include "AsArray.h"

namespace Swf {
	void AsArray::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObject::construct( _runtime, _numParams, _params );
			
		for( int i=0; i < _numParams; ++i ) {
			std::ostringstream stream;
			stream << i;
			put( stream.str(), _params[i] );
		}
	}
		
	AsObjectHandle AsArray::constructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObjectHandle obj = CORE_NEW AsArray();
		obj->construct( _runtime, _numParams, _params );
		return obj;
	}
	
} /* Swf */ 
