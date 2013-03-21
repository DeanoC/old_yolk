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
				AsObjectFunction* func = (AsObjectFunction*)(it->second);
				return func->value->call( _runtime, _this, _numParams, _params );
			}
		}
		if( prototype != NULL ) {
			return prototype->callMethodOn( _runtime, _this, _name, _numParams, _params );
		} else {
			return AsObjectHandle( AsObjectUndefined::get() );
		}
	}		
	void AsObject::defineOwnProperty( const std::string& _name, AsObjectHandle _value, 
									bool _writable, bool _enumerable, bool _configurable, bool _args ) {
		// todo do this properly
		put( _name, _value );
	}

	AsObjectHandle AsObject::getOwnProperty( const std::string& _name ) const {
		Swf::AsObject::PropertyMap::const_iterator prop = properties.find( _name );
		if( prop != properties.end() ) {
			return prop->second;
		} else {
			return AsObjectHandle( AsObjectUndefined::get() );
		}
	}

	bool AsObject::hasOwnProperty( const std::string& _name ) const {
		if( properties.find( _name ) != properties.end() ) {
			return true;
		} else {
			return false;
		}
	}

	void AsObject::deleteProperty( const std::string& _name, bool _strict ) {
		CORE_ASSERT( hasProperty( _name ) );
		properties.erase( properties.find( _name ) );
	}

	AsObjectHandle AsObject::getProperty( const std::string& _name ) const {
		auto ownProp = getOwnProperty( _name );
		if(  ownProp != AsObjectUndefined::get() ) {
			return ownProp;
		}else if( prototype != NULL ) {
			return prototype->getProperty( _name );
		} else {
			return AsObjectHandle( AsObjectUndefined::get() );
		}
	}

	void AsObject::put( const std::string& _name, AsObjectHandle _handle, bool _strict ) {
		properties[ _name ] = _handle;
	}

	bool AsObject::hasProperty( const std::string& _name ) const {
		if( getProperty( _name ) == AsObjectUndefined::get() ) {
			return false;
		} else {
			return true;
		}
	}
	
	// get also calls any accessor if specified so it requires the runtime
	AsObjectHandle  AsObject::get( AsAgRuntime* _runtime, const std::string& _name ) const {
		auto prop = AsObject::getProperty( _name );
		if( prop->type() == APT_FUNCTION ) {
			return prop->callMethodOn( _runtime, prop, "()", 0, nullptr );
		} else {
			return prop;
		}
	}
	
	void AsObject::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		// it seems slightly odd to add the constructor function as a callable method but
		// in practise is no different from calling a ctor in C++ manually
		auto ctor = CORE_GC_NEW AsObjectFunction( &AsObject::ctor );
		put( "", ctor );
		put( "()", ctor );

		put( "hasOwnProperty", CORE_GC_NEW AsObjectFunction( &AsObject::hasOwnProperty ) );
		put( "toString", CORE_GC_NEW AsObjectFunction( &AsObject::toString ) );
		put( "toNumber", CORE_GC_NEW AsObjectFunction( &AsObject::toNumber ) );

		for( int i = 0;i < _numParams; ++i ) {
			const std::string name = _params[ i*2 ]->toString();
			AsObjectHandle val = _params[ (i*2)+1 ];
			put( name, val );
		}
	}
	
	AsObjectHandle AsObject::ctor( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		construct( _runtime, _numParams, _params );
		return this; // ??
	}

	AsObjectHandle AsObject::hasOwnProperty( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 1 );
		return CORE_GC_NEW AsObjectBool( hasOwnProperty(_params[0]->toString()) );
	}
	
	AsObjectHandle AsObject::toString( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 0 );
		return CORE_GC_NEW AsObjectString( toString() );
	}
	
	AsObjectHandle AsObject::toNumber( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 0 );
		return CORE_GC_NEW AsObjectNumber( toNumber() );
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

	//=-=-=-=
	// BOOL
	//=-=-=-=
	void AsObjectBool::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		prototype = s_objectPrototype;
		if( _numParams >= 1 ) {
			value = _params[0]->toBoolean();
		}
	}	
	
	//=-=-=-=
	// STRING
	//=-=-=-=
	void AsObjectString::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		prototype = s_objectPrototype;
		put( "length", CORE_GC_NEW AsObjectFunction( &AsObjectString::length ) );
		if( _numParams >= 1 ) {
			value = _params[0]->toString();
		}
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
			return CORE_GC_NEW AsObjectNumber( (int)value.length() );
		} else {
			return AsObject::getProperty( _name );
		}
	}
	
	AsObjectHandle AsObjectString::length( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params ) {
		assert(_numParams == 0);
		return AsObjectHandle( CORE_GC_NEW AsObjectNumber( (int)value.length() ) );
	}
	
	double AsObjectString::toNumber() const {
		std::istringstream stream(value);
		double d;
		stream >> d;
		return d;
	}		
	
	//=-=-=-=
	// NUMBER
	//=-=-=-=
	void AsObjectNumber::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		prototype = s_objectPrototype;
		if( _numParams >= 1 ) {
			value = _params[0]->toNumber();
		}
	}	
	std::string AsObjectNumber::toString() const {
		std::ostringstream stream;
		stream << value;
		return stream.str();
	}
	

}