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
} /* Core */ 
