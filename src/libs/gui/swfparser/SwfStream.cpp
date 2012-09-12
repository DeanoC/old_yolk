// 
//  SwfStream.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "EndianBitConverter.h"
#include "SwfStream.h"
#include "core/FileIO.h"
#include <vector>

namespace Swf{
	const EndianBitConverter& SwfStream::endianConverter = EndianBitConverter::CreateForLittleEndian();
	
	SwfStream::SwfStream( Core::InOutInterface* _stream ) {
		stream = _stream;
		align();
	}
	
	SwfStream::SwfStream() {
		stream = NULL;
	}
	
	void SwfStream::setStream( Core::InOutInterface* _file ) {
		stream = _file;
	}
	
	void SwfStream::setToMarker(long _marker) {
		stream->seekFromStart( _marker);
	}
	
	long SwfStream::marker() {
		return stream->tell();
	}
	
	uint32_t SwfStream::readUInt(int _bitcount) {
		assert(_bitcount <= 32 && _bitcount >= 0);
		unsigned int value = 0;
		int bits_needed = _bitcount;
		while (bits_needed > 0) {
			if (unusedBits > 0) {
				if (bits_needed >= unusedBits) {
					value |= (currentByte << (bits_needed - unusedBits));
					bits_needed -= unusedBits;
					currentByte = 0;
					unusedBits = 0;
				} else {
					value |= (currentByte >> (unusedBits - bits_needed));
					currentByte &= (uint8_t)((1 << (unusedBits - bits_needed)) - 1);
					unusedBits -= bits_needed;
					bits_needed = 0;
				}
			} else {
				currentByte = stream->getByte();
				unusedBits = 8;
			}
		}
		return value;
	}
	std::string SwfStream::readLengthString() {
		align();
		uint8_t len = readUInt8();
		static const int MAX_LENGTH = 1024; // max 255 + overrun buffer space...
		uint8_t array[MAX_LENGTH];
		stream->read(array,len);
		array[len] = 0;
		return std::string( (const char*)array);
	}

	std::string SwfStream::readString() {
		align();
		
		std::vector<uint8_t> tempString;
		
		uint8_t c = 0;
		do {
			c = readUInt8();
			if (c != 0) {
				tempString.push_back(c);
			}
		} while (c != 0);
		tempString.push_back(0);
		return std::string( (const char*) &tempString[0]);
	}
}

