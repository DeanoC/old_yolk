// 
//  SwfImage.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfImage.h"
#include "SwfRGB.h"

namespace Swf
{
	SwfClutImage::SwfClutImage(int _width, int _height, bool _rgb, int _numClutEntries, uint8_t* _clut, uint8_t* _image) :
		SwfImage( _width, _height ) {
		// does clut contain alpha data?
		clut = CORE_NEW_ARRAY SwfRGBA[_numClutEntries];
		if (_rgb) {
			// no alpha in clut so pretend
			for (int i = 0; i < _numClutEntries; ++i) {
				clut[i] = SwfRGBA( 	_clut[(i * 3) + 0], 
											_clut[(i * 3) + 1], 
											_clut[(i * 3) + 2], 
											(uint8_t)255 );
			}
		} else {
			for (int i = 0; i < _numClutEntries; ++i) {
				clut[i] = SwfRGBA(	_clut[(i * 4) + 0], 
											_clut[(i * 4) + 1], 
											_clut[(i * 4) + 2], 
											_clut[(i * 4) + 3] );
			}
		}
		image = CORE_NEW_ARRAY uint8_t[Width() * Height()];
		uint8_t* src = _image;
		uint8_t* dst = image;
		for( int i = 0; i < _height;++i ) {
			std::memcpy(dst, src, Width() );
			src += Core::alignTo(Width(), 4);
			dst += Width();
		}
	}
	
	Swf1555Image::Swf1555Image(int _width, int _height, int rowStride, uint8_t* _image) :
		SwfImage(_width, _height) {
		image = CORE_NEW_ARRAY SwfRGBA[Width() * Height()];
		for (int y = 0; y < Height(); ++y) {
			for (int x = 0; x < Width(); ++x) {
				union {
					struct {
						uint8_t a;
						uint8_t b;
					} bytes;
					uint16_t a; 
				} conv;
				conv.bytes.a = _image[ (y * rowStride) + (x * 2) + 0];
				conv.bytes.b = _image[ (y * rowStride) + (x * 2) + 1];
				uint16_t col = conv.bytes.a;

				uint8_t r, g, b, a;
				r = (uint8_t)((col & 0x7C00) >> 0);
				g = (uint8_t)((col & 0x02E0) >> 5);
				b = (uint8_t)((col & 0x001F) >> 10);
				a = (uint8_t)((col & 0x8000) >> 15);

				image[(y * Width()) + x] = SwfRGBA(r, g, b, a);
			}
		}
	}

	SwfRGBAImage::SwfRGBAImage(int _width, int _height, bool _rgb, int pixStride, int rowStride, uint8_t* _image) :
		SwfImage(_width, _height) {
		image = CORE_NEW_ARRAY SwfRGBA[Width() * Height()];
		for (int y = 0; y < Height(); ++y) {
			for (int x = 0; x < Width(); ++x) {
				uint8_t r,g,b,a;
				if (_rgb) {
					a = 255;
					int offset = 0;
					// first byte is ignored
					if(pixStride == 4) {
						offset = 1;
					}

					r = _image[(y * rowStride) + (x * pixStride) + offset + 0];
					g = _image[(y * rowStride) + (x * pixStride) + offset + 1];
					b = _image[(y * rowStride) + (x * pixStride) + offset + 2];
				} else {
					r = _image[(y * rowStride) + (x * pixStride) + 0];
					g = _image[(y * rowStride) + (x * pixStride) + 1];
					b = _image[(y * rowStride) + (x * pixStride) + 2];
					a = _image[(y * rowStride) + (x * pixStride) + 3];
				}
				image[(y * Width()) + x] = SwfRGBA(r, g, b, a);
			}
		}
	}
    
} /* Swf */
