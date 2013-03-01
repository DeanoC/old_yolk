/*
 *  AsDate.cpp
 *  SwfPreview
 *
 *  Created by Deano on 27/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui/swfruntime/swfruntime.h"
#include <time.h>
#include "gui/swfruntime/utils.h"
#include "../AsObjectFactory.h"
#include "../AsAgRuntime.h"
#include "../AsObjectFunction.h"
#include "AsDate.h"

namespace Swf {
	void AsDate::construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObject::construct( _runtime, _numParams, _params );

		setProperty( "getHours", CORE_NEW AsObjectThisFunction( &AsDate::getHours ) );
		setProperty( "getMinutes", CORE_NEW AsObjectThisFunction( &AsDate::getMinutes ) );
		setProperty( "getSeconds", CORE_NEW AsObjectThisFunction( &AsDate::getSeconds ) );
		setProperty( "getDay", CORE_NEW AsObjectThisFunction( &AsDate::getDay ) );
		setProperty( "getDate", CORE_NEW AsObjectThisFunction( &AsDate::getDate ) );
		setProperty( "getMonth", CORE_NEW AsObjectThisFunction( &AsDate::getMonth ) );
		setProperty( "getFullYear", CORE_NEW AsObjectThisFunction( &AsDate::getFullYear ) );

	}
		
	AsObjectHandle AsDate::constructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		AsObjectHandle obj = CORE_NEW AsDate();
		obj->construct( _runtime, _numParams, _params );
		return obj;
	}
		
	AsObjectHandle AsDate::getHours( AsAgRuntime* _runtime, int _numParams,  AsObjectHandle* _params ) {
		assert(_numParams == 0);
		time_t t = time(0);
		struct tm* lt = localtime(&t);
		return AsObjectHandle( CORE_NEW AsObjectNumber((float)lt->tm_hour) );
	}
		
	AsObjectHandle AsDate::getMinutes( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 0);
		time_t t = time(0);
		struct tm* lt = localtime(&t);
		return AsObjectHandle( CORE_NEW AsObjectNumber((float)lt->tm_min) );
	}
		
	AsObjectHandle AsDate::getSeconds( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 0);
		time_t t = time(0);
		struct tm* lt = localtime(&t);
		return AsObjectHandle( CORE_NEW AsObjectNumber((float)lt->tm_sec) );
	}
	AsObjectHandle AsDate::getDay( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 0);
		time_t t = time(0);
		struct tm* lt = localtime(&t);
		return AsObjectHandle( CORE_NEW AsObjectNumber((float)lt->tm_wday) );
	}
	AsObjectHandle AsDate::getDate( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 0);
		time_t t = time(0);
		struct tm* lt = localtime(&t);
		return AsObjectHandle( CORE_NEW AsObjectNumber((float)lt->tm_mday) );
	}
	AsObjectHandle AsDate::getMonth( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 0);
		time_t t = time(0);
		struct tm* lt = localtime(&t);
		return AsObjectHandle( CORE_NEW AsObjectNumber((float)lt->tm_mon) );
	}
	AsObjectHandle AsDate::getFullYear( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
		assert(_numParams == 0);
		time_t t = time(0);
		struct tm* lt = localtime(&t);
		return AsObjectHandle( CORE_NEW AsObjectNumber((float)lt->tm_year+1900) );
	}
} /* Swf */ 

