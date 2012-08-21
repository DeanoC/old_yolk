// 
//  FileIO.h
//  deano
//  
//  Created by Deano on 2008-10-12.
//  Copyright 2008 Cloud Pixies Ltd. All rights reserved.
//

#pragma once

#ifndef FILEIO_H_SCRCPCW3
#define FILEIO_H_SCRCPCW3

 
#include "core/core.h"
#include <cstdio>


namespace Core {


class InOutInterface {
public:
	virtual ~InOutInterface(){}
		
	virtual void close() = 0;
	virtual bool isValid() const = 0;
		
	virtual uint64_t read(uint8_t* _buffer, uint64_t _bytes) = 0;
		
	virtual uint64_t tell() = 0;
	virtual void seekFromStart( uint64_t _seek ) = 0;
	virtual uint64_t bytesLeft() = 0;
	virtual uint8_t getByte() = 0;

	virtual InOutInterface& inOut() { return (InOutInterface&)(*this); }

	// these help hook the stb_image functions to the these interfaces
	static int C_read( void* user, char* data, int size ) {
		InOutInterface* io = (InOutInterface*) user;
		return (int) io->read( (uint8_t*) data, (uint64_t) size );
	}
	static void C_skip( void* user, unsigned int n ) {
		InOutInterface* io = (InOutInterface*) user;
		uint64_t c = io->tell();
		io->seekFromStart( c + n );
	}
	static int C_eof( void* user ) {
		InOutInterface* io = (InOutInterface*) user;
		return io->bytesLeft() == 0;
	}

};


class File : public InOutInterface {
public:
	explicit File() : fh(NULL) {}
	explicit File( const char* _path ) : fh(NULL) {
		open(_path);
	}
	~File() {
		close();
	}
		
	bool open( const char* _path );
	bool openText( const char* _path );
	bool createNew( const char* _path );

	virtual void close();
	virtual bool isValid() const {
		return (fh != NULL);
	}
		
	virtual uint64_t read(uint8_t* _buffer, uint64_t _len);
	virtual uint64_t write(uint8_t* _buffer, uint64_t _len);

	virtual uint64_t tell();
	virtual void seekFromStart( uint64_t _seek );		
	virtual uint64_t bytesLeft();
	virtual uint8_t getByte();

	// for when you need to things like fstat etc. not supported via the usual interface
	FILE* getFH() const { return fh; }
protected:
	FILE* fh;
};


class MemFile : public InOutInterface {
public:
	explicit MemFile() : buffer(NULL), size(0), offset(0) {}
	explicit MemFile( const char* _path );
	explicit MemFile( File& file );

	explicit MemFile( size_t _size ) {
		buffer = CORE_NEW_ARRAY uint8_t[_size];
		size = _size;
		offset = 0;		
	}
	explicit MemFile( uint8_t* _mem, size_t _size ){
		buffer = _mem;
		size = _size;
		offset = 0;
	}
	bool loadFile( const char* _path );
	bool loadTextFile( const char* _path );

	uint8_t* getBuffer() { return buffer; }

	virtual void close(){
		CORE_DELETE_ARRAY buffer;
		buffer = 0;
	}

	virtual bool isValid() const {
		return (buffer != NULL);
	}	

	virtual uint64_t read( uint8_t* _buffer, uint64_t _len ){
		if ( offset + _len > size ) {
			_len = size - offset;
		}
			
		memcpy( _buffer, buffer+offset, size_t(_len) );
		offset += _len;
		return _len;
	}

	virtual uint64_t tell() {
		return offset;
	}
	virtual void seekFromStart( uint64_t _seek ) {
		offset = _seek;
	}
	virtual uint8_t getByte() {
		if( offset > size)
			return 0;
		if( offset < 0 )
			return 0;
				
		return buffer[ offset++ ];
	}
	virtual uint64_t bytesLeft() {
		return size - offset;
	}

	InOutInterface& inOut() { return (InOutInterface&)(*this); }

	uint8_t* takeBufferOwnership() { 
		uint8_t* tmp = buffer;
		buffer = NULL; size = 0; offset = 0;
		return tmp;
	}
				 
protected:
	uint8_t*	buffer;
	uint64_t	offset;
	size_t 	size;
};


}	//namespace Core


#endif
