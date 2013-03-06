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
		AsObject::s_objectPrototype = CORE_GC_NEW AsObject();
		AsObject::s_objectPrototype->construct( _runtime, 0, NULL );
		AsObjectString::s_stringPrototype = CORE_GC_NEW AsObjectString("");
		AsObjectString::s_stringPrototype->construct( _runtime, 0, NULL );			

		// install standard constructor functions
		registerFunc( "object", &AsObjectFactory::objectConstructFunction );
		registerFunc( "bool", &AsObjectFactory::boolConstructFunction );
		registerFunc( "string", &AsObjectFactory::stringConstructFunction );
		registerFunc( "number", &AsObjectFactory::numberConstructFunction );
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

	AsObjectHandle AsObjectFactory::objectConstructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObjectHandle a = CORE_GC_NEW AsObject();
		a->construct( _runtime, _numParams, _params );
		return a;
	}
	AsObjectHandle AsObjectFactory::boolConstructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObjectHandle a = CORE_GC_NEW AsObjectBool( false );
		a->construct( _runtime, _numParams, _params );
		return a;
	}

	AsObjectHandle AsObjectFactory::stringConstructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObjectHandle a = CORE_GC_NEW AsObjectString( "" );
		a->construct( _runtime, _numParams, _params );
		return a;
	}
	AsObjectHandle AsObjectFactory::numberConstructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObjectHandle a = CORE_GC_NEW AsObjectNumber( 0.0 );
		a->construct( _runtime, _numParams, _params );
		return a;
	}

}