// 
//  SwfFillStyle.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfFillStyle.h"
#include "SwfMatrix.h"
#include "SwfStream.h"
#include "SwfRGB.h"
#include "SwfGradient.h"
namespace Swf
{
	SwfFillStyle* SwfFillStyle::Read(SwfStream& _stream, bool _shape3orMore)
	{
		FillType type = (FillType)_stream.readUInt8();
		if (type == SolidFill) {
			SwfRGBA colour = _shape3orMore ? SwfRGBA::ReadRGBA(_stream) : SwfRGBA::ReadRGB(_stream);
			return CORE_NEW SwfSolidFillStyle(colour);
		}
		if( type == LinearGradientFill || 
			type == RadialGradientFill || 
			type == FocalRadialGradientFill ) {
				SwfMatrix* matrix = SwfMatrix::Read(_stream);
				SwfGradient* gradient = SwfGradient::Read(_stream, _shape3orMore);
			if (type == FocalRadialGradientFill) {
				float focalPt = _stream.readFixed8();
				return CORE_NEW SwfFocalGradientFillStyle(type, matrix, gradient,focalPt);
			} else {
				return CORE_NEW SwfGradientFillStyle(type, matrix, gradient);
			}
		}
		if (type == ClippedBitmapFill || 
			type == NonSmoothedClippedBitmapFill || 
			type == RepeatingBitmapFill || 
			type == NonSmoothedRepeatingBitmapFill) {
			uint16_t id = _stream.readUInt16();
			SwfMatrix* matrix = SwfMatrix::Read(_stream);
			return CORE_NEW SwfBitmapFillStyle(type, id, matrix);
		}
		// unknown fill type...
		return NULL;
	}
	SwfFillStyle* SwfFillStyle::ReadMorph(SwfStream& _stream ) {
		FillType type = (FillType)_stream.readUInt8();
		FillType morphType = (FillType)(((uint8_t)type) + 0x100);
		if (type == SolidFill) {
			SwfRGBA startColour = SwfRGBA::ReadRGBA(_stream);
			SwfRGBA endColour = SwfRGBA::ReadRGBA(_stream);
			return CORE_NEW SwfMorphSolidFillStyle(startColour, endColour);
		}
		if (type == LinearGradientFill ||
			type == RadialGradientFill ||
			type == FocalRadialGradientFill) {
			SwfMatrix* startMatrix = SwfMatrix::Read(_stream);
			SwfMatrix* endMatrix = SwfMatrix::Read(_stream);
			SwfGradient* startGradient = SwfGradient::ReadMorph(_stream);
			SwfGradient* endGradient = SwfGradient::ReadMorph(_stream);
			if (type == FocalRadialGradientFill) {
				assert(false);// morphs can't be focal radial accoring to the docs...
				float focalPt = _stream.readFixed8();
				return CORE_NEW SwfFocalGradientFillStyle(type, startMatrix, startGradient, focalPt);
			} else {
				return CORE_NEW SwfMorphGradientFillStyle(morphType, startMatrix, startGradient, endMatrix, endGradient);
			}
		}
		if (type == ClippedBitmapFill ||
			type == NonSmoothedClippedBitmapFill ||
			type == RepeatingBitmapFill ||
			type == NonSmoothedRepeatingBitmapFill) {
			uint16_t id = _stream.readUInt16();
			SwfMatrix* startMatrix = SwfMatrix::Read(_stream);
			SwfMatrix* endMatrix = SwfMatrix::Read(_stream);
			return CORE_NEW SwfMorphBitmapFillStyle(morphType, id, startMatrix, endMatrix);
		}
		// unknown fill type...
		return NULL;
	}    
} /* Swf */
