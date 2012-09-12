/*
 *  AsDate.cpp
 *  SwfPreview
 *
 *  Created by Deano on 27/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "gui/swfruntime/swfruntime.h"
#include "../autogen/AsObjectFactory.h"
#include "../autogen/AsAgRuntime.h"
#include "../autogen/AsObjectFunction.h"
#include "gui/SwfRuntime/SwfRuntimeUtils.h"
#include <time.h>
#include "AsDate.h"

namespace Swf {
	namespace AutoGen {
		void AsDate::Construct( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
			AsObject::Construct( _runtime, _numParams, _params );

			SetProperty( "getHours", CORE_NEW AsObjectThisFunction( &AsDate::getHours ) );
			SetProperty( "getMinutes", CORE_NEW AsObjectThisFunction( &AsDate::getMinutes ) );
			SetProperty( "getSeconds", CORE_NEW AsObjectThisFunction( &AsDate::getSeconds ) );
			SetProperty( "getDay", CORE_NEW AsObjectThisFunction( &AsDate::getDay ) );
			SetProperty( "getDate", CORE_NEW AsObjectThisFunction( &AsDate::getDate ) );
			SetProperty( "getMonth", CORE_NEW AsObjectThisFunction( &AsDate::getMonth ) );
			SetProperty( "getFullYear", CORE_NEW AsObjectThisFunction( &AsDate::getFullYear ) );

		}
		
		AsObjectHandle AsDate::ConstructFunction( AsAgRuntime* _runtime, int _numParams, AsObjectHandle* _params ) {
			AsObjectHandle obj = CORE_NEW AsDate();
			obj->Construct( _runtime, _numParams, _params );
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
	} /* AsLib */ 
} /* Swf */ 

