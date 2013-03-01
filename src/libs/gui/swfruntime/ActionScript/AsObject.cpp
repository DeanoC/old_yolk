/*
 *  AsObject.cpp
 *  SwfPreview
 *
 *  Created by Deano on 19/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui/swfruntime/swfruntime.h"
#include "gui/swfruntime/utils.h"
#include "AsFunction.h"
#include "AsObjectFunction.h"
#include "AsAgRuntime.h"
#include "AsObjectFactory.h"
#include "AsObject.h"

#include <sstream>

namespace Swf {
	AsObjectHandle AsObject::s_objectPrototype;
	AsObjectHandle AsObjectString::s_stringPrototype;
	
	AsObjectHandle AsObject::callMethodOn( AsAgRuntime* _runtime, AsObjectHandle _this, const std::string& _name, int _numParams, AsObjectHandle* _params ) {
		// check object members first
		PropertyMap::const_iterator it = properties.find( _name );
		if( it != properties.end() ) {
			if( it->second->type() == APT_FUNCTION ) {
				AsObjectThisFunction* func = (AsObjectThisFunction*)(it->second);
				return func->value->call( _runtime, _this, _numParams, _params );
			}
		}
		if( prototype != NULL ) {
			return prototype->callMethodOn( _runtime, _this, _name, _numParams, _params );
		} else {
			return AsObjectHandle( AsObjectUndefined::get() );
		}
	}

	AsObjectHandle AsObject::getProperty( const std::string& _name ) const {
		Swf::AsObject::PropertyMap::const_iterator prop = properties.find( _name );
		if( prop != properties.end() ) {
			return prop->second;//properties[ _name ];
		} else if( prototype != NULL ) {
			return prototype->getProperty( _name );
		} else {
			return AsObjectHandle( AsObjectUndefined::get() );
		}
	}

	void AsObject::setProperty( const std::string& _name, AsObjectHandle _handle ) {
		properties[ _name ] = _handle;
	}
	
	bool AsObject::hasOwnProperty( const std::string& _name ) {
		if( properties.find( _name ) != properties.end() ) {
			return true;
		} else {
			return false;
		}
	}
	
	void AsObject::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		setProperty( "hasOwnProperty", CORE_NEW AsObjectThisFunction( &AsObject::hasOwnProperty ) );
		setProperty( "toString", CORE_NEW AsObjectThisFunction( &AsObject::toString ) );
		setProperty( "toNumber", CORE_NEW AsObjectThisFunction( &AsObject::toNumber ) );
	}
	
	AsObjectHandle AsObject::hasOwnProperty( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 1 );
		return CORE_NEW AsObjectBool( hasOwnProperty(_params[0]->toString()) );
	}
	
	AsObjectHandle AsObject::toString( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 0 );
		return CORE_NEW AsObjectString( toString() );
	}
	
	AsObjectHandle AsObject::toNumber( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 0 );
		return CORE_NEW AsObjectNumber( toNumber() );
	}
		
	bool AsObject::is( AsObjectHandle _b ) const {
		// Is sees if we are the same type
		// for objects we then check same class
		if( type() == _b->type() ) {
			if(_b->type() == APT_OBJECT ) {
				// TODO check prototypes?
				return true;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}
	
	std::string AsObjectNumber::toString() const {
		std::ostringstream stream;
		stream << value;
		return stream.str();
	}
	
	void AsObjectString::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		prototype = s_objectPrototype;
		setProperty( "length", CORE_NEW AsObjectThisFunction( &AsObjectString::length ) );
	}
	bool AsObjectString::toBoolean() const {
		if( value == "1" || value == "true" ||
			 value == "True" || value == "TRUE" ){
			return true;
		} else {
			return false;
		}
	}
	
	AsObjectHandle  AsObjectString::getProperty( const std::string& _name ) const {
		if( _name == "length" ) {
			return CORE_NEW AsObjectNumber( (int)value.length() );
		} else {
			return AsObject::getProperty( _name );
		}
	}
	
	AsObjectHandle AsObjectString::length( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params ) {
		assert(_numParams == 0);
		return AsObjectHandle( CORE_NEW AsObjectNumber( (int)value.length() ) );
	}
	
	double AsObjectString::toNumber() const {
		std::istringstream stream(value);
		double d;
		stream >> d;
		return d;
	}		
}