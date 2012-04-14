// 
//  FileIO.h
//  deano
//  
//  Created by Deano on 2008-10-12.
//  Copyright 2008 Zombie House Studios. All rights reserved.
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
		
	virtual void Close() = 0;
	virtual bool IsValid() = 0;
		
	virtual uint64_t Read(uint8_t* _buffer, uint64_t _bytes) = 0;
		
	virtual uint64_t Tell() = 0;
	virtual void SeekFromStart( uint64_t _seek ) = 0;
	virtual uint64_t BytesLeft() = 0;
	virtual uint8_t GetByte() = 0;
};


class File : public InOutInterface {
public:
	File() : fh(NULL) {}
	File( const char* _path ) : fh(NULL) {
		Open(_path);
	}
	~File() {
		Close();
	}
		
	bool Open( const char* _path );
	virtual void Close();
	virtual bool IsValid() {
		return (fh != NULL);
	}
		
	virtual uint64_t Read(uint8_t* _buffer, uint64_t _len);

	virtual uint64_t Tell();
	virtual void SeekFromStart( uint64_t _seek );		
	virtual uint64_t BytesLeft();
	virtual uint8_t GetByte();
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
	virtual void Close(){
		CORE_DELETE_ARRAY m_Buffer;
		m_Buffer = 0;
	}
	virtual bool IsValid() {
		return (m_Buffer != NULL);
	}
		

	virtual uint64_t Read( uint8_t* _buffer, uint64_t _len ){
		if ( m_Offset + _len > m_Size ) {
			_len = m_Size - m_Offset;
		}
			
		memcpy( _buffer, m_Buffer+m_Offset, size_t(_len) );
		m_Offset += _len;
		return _len;
	}

	virtual uint64_t Tell() {
		return m_Offset;
	}
	virtual void SeekFromStart( uint64_t _seek ) {
		m_Offset = _seek;
	}
	virtual uint8_t GetByte() {
		if( m_Offset > m_Size)
			return 0;
		if( m_Offset < 0 )
			return 0;
				
		return m_Buffer[ m_Offset++ ];
	}
	virtual uint64_t BytesLeft(){
		return m_Size - m_Offset;
	}
				 
protected:
	uint8_t*	m_Buffer;
	uint64_t	m_Offset;
	uint64_t	m_Size;
};


}	//namespace Core


#endif
