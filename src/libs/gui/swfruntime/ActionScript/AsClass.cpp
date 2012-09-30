/*
 *  AsClass.cpp
 *  SwfPreview
 *
 *  Created by Deano on 25/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "AsAgRuntime.h"
#include "AsClass.h"

namespace Swf {
	void AsClass::registerMemberFunction( const std::string& _name, Class2FuncPtr _func ) {
		std::string str = _name;
		if( runtime->isCaseSensitive() == false ) {
			std::transform(str.begin(), str.end(), str.begin(), tolower);
		}
		memberFunctions[ str ] = _func;
	}

	AsObjectHandle AsClass::callMember( AsObject* _this, const std::string& _name, int _numArgs, AsObjectHandle* _params ) {
		if( memberFunctions.find(_name) != memberFunctions.end() ) {
			Class2FuncPtr func = memberFunctions[_name];
			TODO_ASSERT( false && "berhhh??!" );
//				return (_this->asclass->*func)(_this, _numArgs, _params);
			return nullptr;
		} else {
			return AsObjectHandle( AsObjectUndefined::get() );
		}
	}
}