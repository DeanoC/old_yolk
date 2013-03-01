// 
//  SwfImage.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFIMAGE_H_
#define _SWFIMAGE_H_

#include "SwfRGB.h"

namespace Swf
{
	class SwfImage {
	public:
		SwfImage ();
		virtual ~SwfImage (){};

		int Width() const {
			return width; 
		}
		int Height() const {
			return height;
		}

		virtual uint32_t ToPackedRGBA(int x, int y) = 0;
		virtual uint32_t ToPackedARGB(int x, int y) = 0;
		virtual uint32_t ToPackedABGR(int x, int y) = 0;
	protected:
		SwfImage(int _width, int _height) {
			width = _width;
			height = _height;
		}
	private:
		int width;
		int height;
	};
	
	class SwfClutImage : public SwfImage {
	public:
		SwfRGBA* clut;
		uint8_t* image;

		SwfClutImage(int _width, int _height, bool _rgb, int _numClutEntries, uint8_t* _clut, uint8_t* _image);

		uint32_t ToPackedRGBA(int x, int y) {
			uint8_t index = image[(y * Width()) + x];
			return clut[index].ToPackedRGBA();
		}
		uint32_t ToPackedARGB(int x, int y) {
			uint8_t index = image[(y * Width()) + x];
			return clut[index].ToPackedARGB();
		}
		uint32_t ToPackedABGR(int x, int y) {
			uint8_t index = image[(y * Width()) + x];
			return clut[index].ToPackedABGR();
		}
	};
	
	class Swf1555Image : public SwfImage {
	public:
		SwfRGBA* image;

		Swf1555Image(int _width, int _height, int rowStride, uint8_t* _image);

		uint32_t ToPackedRGBA(int x, int y) {
			return image[(y * Width()) + x].ToPackedRGBA();
		}
		uint32_t ToPackedARGB(int x, int y) {
			return image[(y * Width()) + x].ToPackedARGB();
		}
		uint32_t ToPackedABGR(int x, int y) {
			return image[(y * Width()) + x].ToPackedABGR();
		}
	};
	
	class SwfRGBAImage : public SwfImage {
	public:
		SwfRGBA* image;

		SwfRGBAImage(int _width, int _height, bool _rgb, int pixStride, int rowStride, uint8_t* _image);

		uint32_t ToPackedRGBA(int x, int y) {
			return image[(y * Width()) + x].ToPackedRGBA();
		}
		uint32_t ToPackedARGB(int x, int y) {
			return image[(y * Width()) + x].ToPackedARGB();
		}
		uint32_t ToPackedABGR(int x, int y) {
			return image[(y * Width()) + x].ToPackedABGR();
		}
	};
	
	
} /* Swf */


#endif /* _SWFIMAGE_H_ */
