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
	namespace AutoGen {	
		AsObjectFactory::AsObjectFactory( AsAgRuntime* _runtime ) :
			runtime(_runtime) {
			AsObject::s_objectPrototype = CORE_NEW AsObject();
			AsObject::s_objectPrototype->Construct( _runtime, 0, NULL );
			AsObjectString::s_stringPrototype = CORE_NEW AsObjectString("");
			AsObjectString::s_stringPrototype->Construct( _runtime, 0, NULL );			
		}

		void AsObjectFactory::Register( const std::string& _name, ConstructFunction _ctor ) {
			constructors[ _name ] = _ctor;
		}
		
		AsObjectHandle AsObjectFactory::Construct( const std::string& _name, int _numParams, AsObjectHandle* _params ) {
			if( constructors.find(_name) != constructors.end() ) {
				return AsObjectHandle( (constructors[_name])( runtime, _numParams, _params ) );
			} else {
				return AsObjectHandle( AsObjectUndefined::Get() );
			}
		}
	}
}