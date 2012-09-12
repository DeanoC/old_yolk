/*
 *  AsArray.cpp
 *  SwfPreview
 *
 *  Created by Deano on 07/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "../autogen/AsObjectFactory.h"
#include "../autogen/AsAgRuntime.h"
#include <sstream>
#include "AsArray.h"

namespace Swf {
	namespace AutoGen {
		void AsArray::Construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
			AsObject::Construct( _runtime, _numParams, _params );
			
			for( int i=0; i < _numParams; ++i ) {
				std::ostringstream stream;
				stream << i;
				SetProperty( stream.str(), _params[i] );
			}
		}
		
		AsObjectHandle AsArray::ConstructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
			AsObjectHandle obj = CORE_NEW AsArray();
			obj->Construct( _runtime, _numParams, _params );
			return obj;
		}
	} /* AutoGen */ 
	
} /* Swf */ 
