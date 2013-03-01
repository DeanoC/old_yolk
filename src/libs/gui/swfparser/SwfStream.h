// 
//  SwfStream.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined( YOLK_GUI_SWFPARSER_SWF_STREAM_H_ )
#define YOLK_GUI_SWFPARSER_SWF_STREAM_H_

#include "EndianBitConverter.h"
#include "core/fileio.h"

namespace Swf {
	class SwfStream {
	public:
		SwfStream();
		SwfStream( Core::InOutInterface* _file );

		void setStream( Core::InOutInterface* _file );

		int version;

		void align() {
			currentByte = 0;
			unusedBits = 0;
		}

		uint64_t marker();

		void setToMarker( uint64_t marker );

		void readBytes(uint8_t* _dest, size_t _numBytes) {
			align();
			stream->read(_dest,_numBytes);
		}
		uint32_t readUInt(int _bitcount);

		int32_t readInt(int _bitcount) {
			int32_t value = (int32_t)readUInt(_bitcount);
			if ((value & (1 << (_bitcount - 1))) != 0) {
				value |= -1 << _bitcount;
			}
			return value;
		}
		int8_t readInt8() {
			align();
			stream->read(&currentByte,1);
			return (int8_t)currentByte;
		}
		uint8_t readUInt8() {
			align();
			stream->read(&currentByte,1);
			return (uint8_t)currentByte;
		}
		int16_t readInt16() {
			align();
			uint8_t array[2];
			stream->read(array,2);
			return endianConverter.ToInt16(array);
		}
		uint16_t readUInt16() {
			align();
			uint8_t array[2];
			stream->read(array,2);
			return endianConverter.ToUInt16(array);
		}
		int32_t readInt32() {
			align();
			uint8_t array[4];
			stream->read(array,4);
			return endianConverter.ToInt32(array);
		}
		uint32_t readUInt32() {
			align();
			uint8_t array[4];
			stream->read(array,4);
			return endianConverter.ToUInt32(array);
		}
		int64_t readInt64() {
			align();
			uint8_t array[8];
			stream->read(array,8);
			return endianConverter.ToInt64(array);
		}
		uint64_t readUInt64() {
			align();
			uint8_t array[8];
			stream->read(array,8);
			return endianConverter.ToUInt64(array);
		}
		float readFixed16() {
			align();
			int32_t i = readInt32();
			return (((float)i) / 65536.0f);
		}
		float readFixed8() {
			align();
			int16_t i = readInt16();
			return (((float)i) / 256.0f);
		}

		std::string readLengthString();
		std::string readString();

		bool readFlag() {
			uint8_t bit = (uint8_t) readUInt(1);
			return (bit == 0x1) ? true : false;
		}

		Core::InOutInterface* stream;
	private:
		uint8_t currentByte;
		int unusedBits;
		static const EndianBitConverter& endianConverter;

	};
}

#endif