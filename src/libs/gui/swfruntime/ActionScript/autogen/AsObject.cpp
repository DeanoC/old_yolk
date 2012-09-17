/*
 *  AsObject.cpp
 *  SwfPreview
 *
 *  Created by Deano on 19/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <sstream>
#include "gui/swfruntime/swfruntime.h"
#include "../codegen/AsFunction.h"
#include "gui/swfruntime/utils.h"
#include "AsObjectFunction.h"
#include "AsAgRuntime.h"
#include "AsObjectFactory.h"
#include "AsObject.h"

namespace Swf {
	using namespace CodeGen;
namespace AutoGen {	
	AsObjectHandle AsObject::s_objectPrototype;
	AsObjectHandle AsObjectString::s_stringPrototype;
	
	AsObjectHandle AsObject::CallMethodOn( AsAgRuntime* _runtime, AsObjectHandle _this, const std::string& _name, int _numParams, AsObjectHandle* _params ) {
		// check object members first
		PropertyMap::const_iterator it = properties.find( _name );
		if( it != properties.end() ) {
			if( it->second->Type() == APT_FUNCTION ) {
				AsObjectThisFunction* func = (AsObjectThisFunction*)(it->second);
				return func->value->Call( _runtime, _this, _numParams, _params );
			}
		}
		if( prototype != NULL ) {
			return prototype->CallMethodOn( _runtime, _this, _name, _numParams, _params );
		} else {
			return AsObjectHandle( AsObjectUndefined::Get() );
		}
	}

	AsObjectHandle AsObject::GetProperty( const std::string& _name ) {
		if( properties.find( _name ) != properties.end() ) {
			return properties[ _name ];
		} else if( prototype != NULL ) {
			return prototype->GetProperty( _name );
		} else {
			return AsObjectHandle( AsObjectUndefined::Get() );
		}
	}

	void AsObject::SetProperty( const std::string& _name, AsObjectHandle _handle ) {
		properties[ _name ] = _handle;
	}
	
	bool AsObject::HasOwnProperty( const std::string& _name ) {
		if( properties.find( _name ) != properties.end() ) {
			return true;
		} else {
			return false;
		}
	}
	
	void AsObject::Construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		SetProperty( "hasOwnProperty", CORE_NEW AsObjectThisFunction( &AsObject::hasOwnProperty ) );
		SetProperty( "toString", CORE_NEW AsObjectThisFunction( &AsObject::toString ) );
		SetProperty( "toNumber", CORE_NEW AsObjectThisFunction( &AsObject::toNumber ) );
	}
	
	AsObjectHandle AsObject::hasOwnProperty( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 1 );
		return CORE_NEW AsObjectBool( HasOwnProperty(_params[0]->ToString()) );
	}
	
	AsObjectHandle AsObject::toString( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 0 );
		return CORE_NEW AsObjectString( ToString() );
	}
	
	AsObjectHandle AsObject::toNumber( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert( _numParams == 0 );
		return CORE_NEW AsObjectNumber( ToNumber() );
	}
		
	bool AsObject::Is( AsObjectHandle _b ) {
		// Is sees if we are the same type
		// for objects we then check same class
		if( Type() == _b->Type() ) {
			if(_b->Type() == APT_OBJECT ) {
				// TODO check prototypes?
				return true;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}
	
	std::string AsObjectNumber::ToString() {
		std::ostringstream stream;
		stream << value;
		return stream.str();
	}
	
	void AsObjectString::Construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		prototype = s_objectPrototype;
		SetProperty( "length", CORE_NEW AsObjectThisFunction( &AsObjectString::length ) );
	}
	bool AsObjectString::ToBoolean() {
		if( value == "1" || value == "true" ||
			 value == "True" || value == "TRUE" ){
			return true;
		} else {
			return false;
		}
	}
	
	AsObjectHandle  AsObjectString::GetProperty( const std::string& _name ) {
		if( _name == "length" ) {
			return CORE_NEW AsObjectNumber( (int)value.length() );
		} else {
			return AsObject::GetProperty( _name );
		}
	}
	
	AsObjectHandle AsObjectString::length( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params ) {
		assert(_numParams == 0);
		return AsObjectHandle( CORE_NEW AsObjectNumber( (int)value.length() ) );
	}
	
	double AsObjectString::ToNumber() {
		std::istringstream stream(value);
		double d;
		stream >> d;
		return d;
	}		
}
}