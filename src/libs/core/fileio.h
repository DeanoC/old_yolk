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
	MemFile(){}
	MemFile( uint8_t* _mem, uint64_t _size ){
		m_Buffer = _mem;
		m_Size = _size;
		m_Offset = 0;
	}
	virtual void close(){
		CORE_DELETE_ARRAY m_Buffer;
		m_Buffer = 0;
	}
	virtual bool isValid() const {
		return (m_Buffer != NULL);
	}
		

	virtual uint64_t read( uint8_t* _buffer, uint64_t _len ){
		if ( m_Offset + _len > m_Size ) {
			_len = m_Size - m_Offset;
		}
			
		memcpy( _buffer, m_Buffer+m_Offset, size_t(_len) );
		m_Offset += _len;
		return _len;
	}

	virtual uint64_t tell() {
		return m_Offset;
	}
	virtual void seekFromStart( uint64_t _seek ) {
		m_Offset = _seek;
	}
	virtual uint8_t getByte() {
		if( m_Offset > m_Size)
			return 0;
		if( m_Offset < 0 )
			return 0;
				
		return m_Buffer[ m_Offset++ ];
	}
	virtual uint64_t bytesLeft() {
		return m_Size - m_Offset;
	}
	InOutInterface& inOut() { return (InOutInterface&)(*this); }
				 
protected:
	uint8_t*	m_Buffer;
	uint64_t	m_Offset;
	uint64_t	m_Size;
};


}	//namespace Core


#endif
