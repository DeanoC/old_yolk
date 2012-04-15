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
	bool File::Open( const char* _path ){
		fh = fopen( _path, "rb" );
		return  ( fh != NULL) ? true : false;
	}
	void File::Close(){
		if( fh ) {
			fclose(fh);
			fh = NULL;
		}
	}
	uint64_t File::Read(uint8_t* _buffer, uint64_t _len){
		return fread(_buffer, 1, (size_t)_len, fh);
	}
	
	uint64_t File::Tell(){
		return ftell(fh);
	}
	void File::SeekFromStart(uint64_t _seek) {
		fseek(fh, (long)_seek, SEEK_SET);
	}
	uint8_t File::GetByte() {
		return fgetc(fh);
	}
	
	uint64_t File::BytesLeft() {
		long pos = (long) Tell();
		fseek(fh, 0, SEEK_END);
		long epos = (long) Tell();
		long left = epos - pos;
		SeekFromStart(pos);
		return left;
	}
} /* Core */ 
