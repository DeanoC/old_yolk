// 
//  FileIO.cpp
//  deano
//  
// 

#include "core/core.h"

#include "fileio.h"
#include <cstdio>

namespace Core
{
	bool File::open( const char* _path ){
		fh = fopen( _path, "rb" );
		return  ( fh != NULL) ? true : false;
	}
	bool File::openText( const char* _path ){
		fh = fopen( _path, "rt" );
		return  ( fh != NULL) ? true : false;
	}

	bool File::createNew( const char* _path ) {
		fh = fopen( _path, "wb" );
		return  ( fh != NULL) ? true : false;		
	}

	void File::close(){
		if( fh ) {
			fclose(fh);
			fh = NULL;
		}
	}
	uint64_t File::read(uint8_t* _buffer, uint64_t _len){
		return fread(_buffer, 1, (size_t)_len, fh);
	}

	uint64_t File::write(uint8_t* _buffer, uint64_t _len){
		return fwrite(_buffer, 1, (size_t)_len, fh);
	}

	
	uint64_t File::tell(){
		return ftell(fh);
	}
	void File::seekFromStart(uint64_t _seek) {
		fseek(fh, (long)_seek, SEEK_SET);
	}
	uint8_t File::getByte() {
		return fgetc(fh);
	}
	
	uint64_t File::bytesLeft() {
		long pos = (long) tell();
		fseek(fh, 0, SEEK_END);
		long epos = (long) tell();
		long left = epos - pos;
		seekFromStart(pos);
		return left;
	}

	MemFile::MemFile( File& file ) {
		size = (size_t) file.bytesLeft();
		buffer = CORE_NEW_ARRAY uint8_t[ size ];
		file.read( buffer, size );
		offset = 0;
	}
	MemFile::MemFile( const char* _path ) :
		size( 0 ), buffer( nullptr ), offset( 0 ) {
		loadFile( _path );
	}
	bool MemFile::loadFile( const char* _path ) {
		File file( _path );
		if( file.isValid() ) {
			size = (size_t) file.bytesLeft();
			buffer = CORE_NEW_ARRAY uint8_t[ size ];
			size = file.read( buffer, size );
			offset = 0;
			return true;
		} else {
			return false;
		}
	}
	bool MemFile::loadTextFile( const char* _path ) {
		File file;
		file.openText( _path );
		if( file.isValid() ) {
			size = (size_t) file.bytesLeft() + 1;
			buffer = CORE_NEW_ARRAY uint8_t[ size ];
			size = file.read( buffer, size - 1 ) + 1;
			buffer[ size - 1] = 0; // add end null
			offset = 0;
			return true;
		} else {
			return false;
		}
	}

} /* Core */ 
