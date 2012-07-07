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
};


class File : public InOutInterface {
public:
	File() : fh(NULL) {}
	File( const char* _path ) : fh(NULL) {
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
	MemFile( uint64_t _size ) {
		buffer = CORE_NEW_ARRAY uint8_t[_size];
		size = _size;
		offset = 0;		
	}
	MemFile( uint8_t* _mem, uint64_t _size ){
		buffer = _mem;
		size = _size;
		offset = 0;
	}
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
				 
protected:
	uint8_t*	buffer;
	uint64_t	offset;
	uint64_t	size;
};


}	//namespace Core


#endif
