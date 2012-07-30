#include "meshimport.h"
#include "core/fileio.h"

#include "boost/tokenizer.hpp"
#include "boost/lexical_cast.hpp"
#include "grometxtimp.h"

namespace MeshImport {

// main objects that can exist at the root
GromeTxtImp::ObjectLookup ObjectTable[] = {
	// Objects
	{ "TerrainZone",		&GromeTxtImp::terrainZoneObjectKey },
		{ "name",				&GromeTxtImp::nameVarKey },
		{ "tiles_no_x",			&GromeTxtImp::defaultVariableKey },
		{ "tiles_no_z",			&GromeTxtImp::defaultVariableKey },
		{ "tiles_size_x",		&GromeTxtImp::defaultVariableKey },
		{ "tiles_size_z",		&GromeTxtImp::defaultVariableKey },
		{ "heightfield",		&GromeTxtImp::heightfieldVariableKey },

	{ "ColorLayer",			&GromeTxtImp::skipKey },
		{ "material_name",			&GromeTxtImp::defaultVariableKey },
		{ "color_image_file",		&GromeTxtImp::defaultVariableKey },
		{ "color_texmap_offset",	&GromeTxtImp::defaultVariableKey },
		{ "color_texmap_scale",		&GromeTxtImp::defaultVariableKey },
		{ "color_texmap_horiz_rot",	&GromeTxtImp::defaultVariableKey },
		{ "color_texmap_vert_rot",	&GromeTxtImp::defaultVariableKey },
		{ "color_texmap_spin_rot",	&GromeTxtImp::defaultVariableKey },
		{ "color_texmap_coords",	&GromeTxtImp::defaultVariableKey },

	{ "ObjectContainer",	&GromeTxtImp::skipKey },
		{ "owner",	&GromeTxtImp::defaultVariableKey },

	{ "", 					&GromeTxtImp::skipKey },
};

GromeTxtImp::GromeTxtImp( const std::string& filename ) :
	loadedOkay( true ),
	lineBuffer( CORE_NEW_ARRAY char[LINE_BUFFER_SIZE] ),
	lastLineBuffer( CORE_NEW_ARRAY char[LINE_BUFFER_SIZE] ) {

	FILE* f;

	f = fopen(filename.c_str(),"rt");
	if (f == NULL) {
		loadedOkay = false;
		return;
	}

	objectReader( f, ObjectTable  );

	CORE_DELETE_ARRAY lineBuffer;
	CORE_DELETE_ARRAY lastLineBuffer;

	fclose( f );
}

GromeTxtImp::~GromeTxtImp() {
}

void GromeTxtImp::readLine( FILE* f) {
	if(ungetLastLine == true) {
		strcpy(lineBuffer, lastLineBuffer);
		ungetLastLine = false;
		return;
	}

	lineBuffer[0] = 0;
	fgets(lineBuffer,LINE_BUFFER_SIZE,f);

	strcpy(lastLineBuffer, lineBuffer);

	lineNum++;
}

const GromeTxtImp::ObjectLookup& GromeTxtImp::findKey(const std::string& text, const ObjectLookup* keyArray) const { 
	auto key = keyArray;

	while( !std::string(key->text).empty() ) {
		if(std::string(key->text) == text) {
			return *key;
		}
		key++;
	}

	return *key;
}

void GromeTxtImp::objectReader( FILE *f, const ObjectLookup* keyArray ) { 
	lineNum = 1;

	while (!feof(f) ) {

		// skip empty lines
		do {
			readLine(f);
		} while( lineBuffer[0] == 0xA );

		// get object key
		char* keyText = strtok(lineBuffer," \t\xA\xD");
		if( keyText == 0 || feof(f) ) {
			return;
		}
		// this current object is being closed, so return
		if( keyText[0] == '}' ) {
			objStack.pop();
			return;
		}
		auto key = findKey( keyText, keyArray );

		char* typeText = strtok(0," \t\xA\xD");
		// is this an object or an variable assignment?
		if( typeText == 0 ) {
			readLine(f);
			char* tmp = strtok(lineBuffer," \t\xA\xD");
			CORE_ASSERT( tmp[0] == '{' );
			auto obj = std::make_shared<GromeTxtObject>();
			objects.push_back( obj );	
			objStack.push( obj.get() );
			obj->push_back( GromeTxtVariable( "__GROME_TYPE", boost::any( std::string(keyText) ) ) );
			if( key.func == &GromeTxtImp::skipKey ) {
				defaultObjectKey( f );
			} else {
				(this->*key.func)(f);
			}
		} else {
			CORE_ASSERT( typeText[0] == '=' );
			if( key.func == &GromeTxtImp::skipKey ) {
				// use default named variable handler
				defaultVariableKey( f );
			} else {
				// allows named variable specific behaviour
				(this->*key.func)(f);
			}
		}
	}
}
void GromeTxtImp::skipKey( FILE *f ) {
	do {
		readLine(f);
	} while( !feof(f) && lineBuffer[0] != '}' );
}


void GromeTxtImp::defaultVariableKey( FILE* f ) {
	ungetLastLine = true;
	readLine(f);
	char* keyText = strtok(lineBuffer," \t\xA\xD");
	char* eqText = strtok(0," \t\xA\xD");
	char* dataText = strtok(0,"\xA\xD");

	objStack.top()->push_back( GromeTxtVariable( keyText, boost::any( std::string(dataText) ) ) );
}

void GromeTxtImp::nameVarKey( FILE* f ) {
	ungetLastLine = true;
	readLine(f);
	char* keyText = strtok(lineBuffer," \t\xA\xD");
	CORE_ASSERT( std::string(keyText) == "name" );
	char* eqText = strtok(0," \t\xA\xD");
	CORE_ASSERT( eqText[0] == '=' );
	char* dataText = strtok(0,"\xA\xD");
	if( dataText[0] == '\"' ) dataText++;
	dataText[ strlen(dataText) ] = '\0';

	objStack.top()->push_back( GromeTxtVariable( keyText, boost::any( std::string(dataText) ) ) );
}


void GromeTxtImp::defaultObjectKey( FILE* f ) {
	objectReader( f, ObjectTable );
}

void GromeTxtImp::terrainZoneObjectKey( FILE* f ) {
	objectReader( f, ObjectTable );
}

void GromeTxtImp::heightfieldVariableKey( FILE* f ) {
	ungetLastLine = true;
	readLine(f);
	char* keyText = strtok(lineBuffer," \t\xA\xD");
	char* eqText = strtok(0," \t\xA\xD");
	char* dataText = strtok(0,"\xA\xD");

	std::string str( dataText );
	boost::tokenizer<> token( str );
	std::vector<float> heights;

	for( boost::tokenizer<>::iterator tok = token.begin(); 
			tok != token.end(); 
			++tok ) {
		heights.push_back( boost::lexical_cast<float>(*tok) );
	}
	objStack.top()->push_back( GromeTxtVariable( keyText, 
		boost::any( heights ) ) );
}


}