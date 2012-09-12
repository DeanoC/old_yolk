// 
//  SwfMorphShape.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfMorphShape.h"
#include "SwfStream.h"
#include "SwfShape.h"
#include "SwfFillStyle.h"
#include "SwfLineStyle.h"
#include "SwfRect.h"
namespace Swf
{
	SwfMorphShape* SwfMorphShape::Read(SwfStream& _stream, int _morphVer)
    {
        uint16_t id = _stream.readUInt16();
        SwfMorphShape* morph = CORE_NEW SwfMorphShape(id);
        morph->startRect = SwfRect::Read(_stream);
        morph->endRect = SwfRect::Read(_stream);
        uint32_t offset = _stream.readUInt32();
        long startEdgeOffset = _stream.marker() + offset;
        int fillCount = (int) _stream.readUInt8();
        if (fillCount == 0xFF)
        {
            fillCount = (int)_stream.readUInt16();
        }
        morph->fillStyleArray = CORE_NEW_ARRAY SwfFillStyle*[fillCount];
        for (int i = 0; i < fillCount; ++i)
        {
            morph->fillStyleArray[i] = SwfFillStyle::ReadMorph(_stream);
        }
        int linefillCount = (int)_stream.readUInt8();
        if (linefillCount == 0xFF)
        {
            linefillCount = (int)_stream.readUInt16();
        }
        morph->lineStyleArray = CORE_NEW_ARRAY SwfLineStyle*[linefillCount];
        for (int i = 0; i < linefillCount; ++i)
        {
            morph->lineStyleArray[i] = SwfMorphLineStyle::Read(_stream,_morphVer);
        }
        morph->startEdges = SwfShape::Read(_stream, false, 3);
        _stream.align();
        morph->endEdges = SwfShape::Read(_stream, false, 3);

//        assert(morph->startEdges->shapeRecords.Length == morph->endEdges->shapeRecords.Length);
        return morph;
    }
    
} /* Swf */
