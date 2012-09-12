// 
//  EndianBitConverter.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_ENDIANBITCONVERTER_H)
#define _ENDIANBITCONVERTER_H

namespace Swf {
	class EndianBitConverter {
	public:
		
		/*!
		 @function	CreateForLittleEndian
		 @abstract   Creates an endian converter when the data is in little endian format to start with
		 @discussion if the machine is little endian, this endianconverter is pass thro, if machine is
		 big endian, it will reverse the data
		 @result     an Object you can use to convert endianness of your data
		 */
		static const EndianBitConverter CreateForLittleEndian() {
			return EndianBitConverter((CPU_ENDIANESS == CPU_LITTLE_ENDIAN));
		}
		/*!
		 @function	 CreateForBigEndian
		 @abstract   Creates an endian converter when the data is in big endian format to start with
		 @discussion if the machine is big endian, this endianconverter is pass thro, if machine is
		 little endian, it will reverse the data
		 @result     an Object you can use to convert endianness of your data
		 */
		static const EndianBitConverter CreateForBigEndian() {
			return EndianBitConverter((CPU_ENDIANESS == CPU_BIG_ENDIAN));
		}
		
		EndianBitConverter(const EndianBitConverter& _copy){
			swap = _copy.swap;
		}
		
	private:
		bool swap;
		EndianBitConverter();
		
		EndianBitConverter(bool _swapBytes) :
			swap(_swapBytes){
		}
		
		
		union end_u {
			struct {
				uint8_t a;
				uint8_t b;
				uint8_t c;
				uint8_t d;
				
				uint8_t e;
				uint8_t f;
				uint8_t g;
				uint8_t h;
			} bytes;
			struct {
				uint16_t a;
				uint16_t b;
				
				uint16_t c;
				uint16_t d;
			} shorts;
			struct {
				uint32_t a;
				uint32_t b;
			} ints;			
			uint64_t a;
		};
		
	public:
		int16_t ToInt16(const uint8_t* _data) const {
			return ToInt16(_data, 0);
		}
		int16_t ToInt16(const uint8_t* _data, int _offset) const {
			end_u* conv = (end_u*) (_data + _offset);
			if(swap){
				uint8_t tmp = conv->bytes.a;
				conv->bytes.a = conv->bytes.b;
				conv->bytes.b = tmp;
			}
			return (int16_t) conv->shorts.a;
		}
		uint16_t ToUInt16(const uint8_t* _data) const {
			return ToUInt16(_data, 0);
		}
		uint16_t ToUInt16(const uint8_t* _data, int _offset) const {
			end_u* conv = (end_u*) (_data + _offset);
			if(swap){
				uint8_t tmp = conv->bytes.a;
				conv->bytes.a = conv->bytes.b;
				conv->bytes.b = tmp;
			}
			return (uint16_t) conv->shorts.a;
		}
		int32_t ToInt32(const uint8_t* _data) const {
			return ToInt32(_data, 0);
		}
		int32_t ToInt32(const uint8_t* _data, int _offset) const {
			end_u* conv = (end_u*) (_data + _offset);
			if(swap){
				uint8_t tmp0 = conv->bytes.a;
				uint8_t tmp1 = conv->bytes.b;
				conv->bytes.a = conv->bytes.d;
				conv->bytes.b = conv->bytes.c;
				conv->bytes.c = tmp1;
				conv->bytes.d = tmp0;
			}
			return (int32_t) conv->ints.a;
		}
		uint32_t ToUInt32(const uint8_t* _data) const {
			return ToUInt32(_data, 0);
		}
		uint32_t ToUInt32(const uint8_t* _data, int _offset) const {
			end_u* conv = (end_u*) (_data + _offset);
			if(swap){
				uint8_t tmp0 = conv->bytes.a;
				uint8_t tmp1 = conv->bytes.b;
				conv->bytes.a = conv->bytes.d;
				conv->bytes.b = conv->bytes.c;
				conv->bytes.c = tmp1;
				conv->bytes.d = tmp0;
			}
			return (uint32_t) conv->ints.a;
		}
		int64_t ToInt64(const uint8_t* _data) const {
			return ToInt64(_data, 0);
		}
		int64_t ToInt64(const uint8_t* _data, int _offset) const {
			end_u* conv = (end_u*) (_data + _offset);
			if(swap){
				uint8_t tmp0 = conv->bytes.a;
				uint8_t tmp1 = conv->bytes.b;
				uint8_t tmp2 = conv->bytes.c;
				uint8_t tmp3 = conv->bytes.d;
				conv->bytes.a = conv->bytes.h;
				conv->bytes.b = conv->bytes.g;
				conv->bytes.c = conv->bytes.f;
				conv->bytes.d = conv->bytes.e;
				conv->bytes.e = tmp3;
				conv->bytes.f = tmp2;
				conv->bytes.g = tmp1;
				conv->bytes.h = tmp0;
			}
			return (int64_t) conv->a;
		}
		uint64_t ToUInt64(const uint8_t* _data) const {
			return ToUInt64(_data, 0);
		}
		uint64_t ToUInt64(const uint8_t* _data, int _offset) const {
			end_u* conv = (end_u*) (_data + _offset);
			if(swap){
				uint8_t tmp0 = conv->bytes.a;
				uint8_t tmp1 = conv->bytes.b;
				uint8_t tmp2 = conv->bytes.c;
				uint8_t tmp3 = conv->bytes.d;
				conv->bytes.a = conv->bytes.h;
				conv->bytes.b = conv->bytes.g;
				conv->bytes.c = conv->bytes.f;
				conv->bytes.d = conv->bytes.e;
				conv->bytes.e = tmp3;
				conv->bytes.f = tmp2;
				conv->bytes.g = tmp1;
				conv->bytes.h = tmp0;
			}
			return (uint64_t) conv->a;
		}
	};
}
#endif