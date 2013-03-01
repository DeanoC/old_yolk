// 
//  SwfLineStyle.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-24.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfLineStyle.h"
#include "SwfRGB.h"
#include "SwfStream.h"
#include "SwfFillStyle.h"
namespace Swf
{
	SwfLineStyle::SwfLineStyle(){
        startCapStyle = CS_Round;
        endCapStyle = CS_Round;
        joinStyle = JS_Round;

        hasFill = false;
        noHScale = false;
        noVScale = false;
        pixelHint = false;
        noClose = false;
        miterLimitFactor = 0.0f;		
	}
	SwfLineStyle* SwfLineStyle::Read(SwfStream& _stream, int _shapeVer)
    {
        SwfLineStyle* lineStyle = CORE_NEW SwfLineStyle();
        if ( _shapeVer < 4)
        {
            lineStyle->width = (int)_stream.readUInt16();
            lineStyle->colour = (_shapeVer >=3) ? SwfRGBA::ReadRGBA(_stream) : SwfRGBA::ReadRGB(_stream);
            return lineStyle;
        }
        else
        {
            lineStyle->width = (int)_stream.readUInt16();
            lineStyle->startCapStyle = (CapStyle)_stream.readUInt(2);
            lineStyle->joinStyle = (JoinStyle)_stream.readUInt(2);
            lineStyle->hasFill = _stream.readFlag();
            lineStyle->noHScale = _stream.readFlag();
            lineStyle->noVScale = _stream.readFlag();
            lineStyle->pixelHint = _stream.readFlag();
            _stream.readUInt(5); // reserved and ignored
            lineStyle->endCapStyle = (CapStyle)_stream.readUInt(2);
            if (lineStyle->joinStyle == JS_Miter)
            {
                lineStyle->miterLimitFactor = _stream.readFixed8();
            }
            if (lineStyle->hasFill == false)
            {
                lineStyle->colour = SwfRGBA::ReadRGBA(_stream);
            }
            else
            {
                lineStyle->fillStyle = SwfFillStyle::Read(_stream, true);
            }
            return lineStyle;
        }
    }
    SwfMorphLineStyle* SwfMorphLineStyle::Read(SwfStream& _stream, int _morphVer)
    {
		_stream.align();
        SwfMorphLineStyle* lineStyle = CORE_NEW SwfMorphLineStyle();
        if (_morphVer < 2)
        {
            lineStyle->width = (int)_stream.readUInt16();
            lineStyle->endWidth = (int)_stream.readUInt16();
            lineStyle->colour = SwfRGBA::ReadRGBA(_stream);
            lineStyle->endColour = SwfRGBA::ReadRGBA(_stream);
            return lineStyle;
        }
        else
        {
            lineStyle->width = (int)_stream.readUInt16();
            lineStyle->endWidth = (int)_stream.readUInt16();
            lineStyle->startCapStyle = (CapStyle)_stream.readUInt(2);
            lineStyle->joinStyle = (JoinStyle)_stream.readUInt(2);
            lineStyle->hasFill = _stream.readFlag();
            lineStyle->noHScale = _stream.readFlag();
            lineStyle->noVScale = _stream.readFlag();
            lineStyle->pixelHint = _stream.readFlag();
            _stream.readUInt(5); // reserved and ignored
            lineStyle->endCapStyle = (CapStyle)_stream.readUInt(2);
            if (lineStyle->joinStyle == JS_Miter)
            {
                lineStyle->miterLimitFactor = _stream.readFixed8();
            }
            if (lineStyle->hasFill == false)
            {
                lineStyle->colour = SwfRGBA::ReadRGBA(_stream);
                lineStyle->endColour = SwfRGBA::ReadRGBA(_stream);
            }
            else
            {
                lineStyle->fillStyle = SwfFillStyle::ReadMorph(_stream);
            }
            return lineStyle;
        }
    }

    
} /* Swf */
