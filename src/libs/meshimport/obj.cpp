// obj.cpp
//

#include "meshimport.h"
#include <iostream>
#include <fstream>

#include <meshmod/materialparameter.h>
#include <meshmod/materialvertexbindings.h>
#include <meshmod/mesh.h>
#include <meshmod/scene.h>
#include <meshmod/sceneobject.h>
#include <meshmod/uvvertex.h>
#include <meshmod/rgbmaterialparameter.h>
#include <meshmod/scalarmaterialparameter.h>
#include <meshmod/texturematerialparameter.h>

namespace {

inline bool TryParseLong( char*& buffer, long& val ) {
	char* end = 0;
	val = strtol( buffer, &end, 10 );
	if( end == buffer )
		return false;
	buffer = end;
	return true;
}

inline long ParseLong( char*& buffer ) {
	char* end = 0;
	long val = strtol( buffer, &end, 10 );
	if( end == buffer ) {
		CoreThrowException( FileError, "unable to parse integer" );
	}
	buffer = end;
	return val;
}

inline bool ParseBool( char*& buffer ) {
	long val = 0;
	if( TryParseLong(buffer, val) == false ) {
		if( strncmp(buffer," off",4)==0 ) {
			buffer+=3;
			return false;
		}
		if( strncmp(buffer," on",3)==0 ) {
			buffer+=2;
			return false;
		}
		return false;
	} else {
		if( val == 0 ) {
			return false;
		}
		else {
			return true;
		}
	}
}

inline void SkipBackslash( char*& buffer ) {
	// find the next backslash by skipping whitespace
	while( *buffer != '/' ) {
		if( *buffer != ' ' && *buffer != '\t' ) {
			CoreThrowException( FileError, "malformed index tuple" );
		}
		++buffer;			
	}

	// and skip it
	++buffer;
}

inline float ParseFloat( char*& buffer ) {
	char* end = 0;
	double val = strtod( buffer, &end );
	if( end == buffer ) {
		CoreThrowException( FileError, "unable to parse floating-point number" );
	}
	buffer = end;
	return float( val );
}

}

