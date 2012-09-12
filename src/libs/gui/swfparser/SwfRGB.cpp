// 
//  SwfRGB.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfStream.h"
#include "SwfRGB.h"

namespace Swf{
	uint32_t SwfRGBA::ToPackedRGBA() {
		assert( a <= 1.0f );
		assert( r <= 1.0f );
		assert( g <= 1.0f );
		assert( b <= 1.0f );
		
		return ( ((uint32_t)(r * 255.0f) << 24) |
					((uint32_t)(g * 255.0f) << 16) |
					((uint32_t)(b * 255.0f) <<  8) |
					((uint32_t)(a * 255.0f) <<  0) );
	}

	uint32_t SwfRGBA::ToPackedARGB() {
		assert( a <= 1.0f );
		assert( r <= 1.0f );
		assert( g <= 1.0f );
		assert( b <= 1.0f );

		return (	((uint32_t)(a * 255.0f) << 24) |
					((uint32_t)(r * 255.0f) << 16)|
					((uint32_t)(g * 255.0f) << 8) |
					((uint32_t)(b * 255.0f) << 0));
	}
	
	uint32_t SwfRGBA::ToPackedABGR() {
		assert( a <= 1.0f );
		assert( r <= 1.0f );
		assert( g <= 1.0f );
		assert( b <= 1.0f );

		return (	((uint32_t)(a * 255.0f) << 24) |
					((uint32_t)(b * 255.0f) << 16)|
					((uint32_t)(g * 255.0f) << 8) |
					((uint32_t)(r * 255.0f) << 0));
	}

	SwfRGBA SwfRGBA::Lerp(const SwfRGBA& _a, const SwfRGBA& _b, float t) {
		assert( t <= 1.0f );

		float it = 1.0f - t;
		return SwfRGBA(		_a.r * t + _b.r * it,
									_a.g * t + _b.g * it,
									_a.b * t + _b.b * it,
									_a.a * t + _b.a * it);
	}

	SwfRGBA SwfRGBA::ReadRGB(SwfStream& _stream) {
		uint8_t br = _stream.readUInt8();
		uint8_t bg = _stream.readUInt8();
		uint8_t bb = _stream.readUInt8();
		return SwfRGBA(br, bg, bb, (uint8_t)255);
	}

	SwfRGBA SwfRGBA::ReadRGBA(SwfStream& _stream) {
		uint8_t br = _stream.readUInt8();
		uint8_t bg = _stream.readUInt8();
		uint8_t bb = _stream.readUInt8();
		uint8_t ba = _stream.readUInt8();
		return SwfRGBA(br, bg, bb, ba);
	}
	
	SwfRGBA  SwfRGBA::ReadARGB(SwfStream& _stream) {
		uint8_t ba = _stream.readUInt8();
		uint8_t br = _stream.readUInt8();
		uint8_t bg = _stream.readUInt8();
		uint8_t bb = _stream.readUInt8();
		return SwfRGBA(br, bg, bb, ba);
	}
	
}