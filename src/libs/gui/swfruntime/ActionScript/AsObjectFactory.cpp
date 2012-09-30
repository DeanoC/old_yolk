/*
 *  AsObjectFactory.cpp
 *  SwfPreview
 *
 *  Created by Deano on 25/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "AsObjectFactory.h"

namespace Swf {
	AsObjectFactory::AsObjectFactory( AsAgRuntime* _runtime ) :
		runtime(_runtime) {
		AsObject::s_objectPrototype = CORE_NEW AsObject();
		AsObject::s_objectPrototype->construct( _runtime, 0, NULL );
		AsObjectString::s_stringPrototype = CORE_NEW AsObjectString("");
		AsObjectString::s_stringPrototype->construct( _runtime, 0, NULL );			
	}

	void AsObjectFactory::registerFunc( const std::string& _name, ConstructFunction _ctor ) {
		constructors[ _name ] = _ctor;
	}
		
	AsObjectHandle AsObjectFactory::construct( const std::string& _name, int _numParams, AsObjectHandle* _params ) {
		if( constructors.find(_name) != constructors.end() ) {
			return AsObjectHandle( (constructors[_name])( runtime, _numParams, _params ) );
		} else {
			return AsObjectHandle( AsObjectUndefined::get() );
		}
	}
}