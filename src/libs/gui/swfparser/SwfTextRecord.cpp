// 
//  SwfTextRecord.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfTextRecord.h"
#include "SwfStream.h"

namespace Swf
{
	SwfTextRecord* SwfTextRecord::Read(SwfStream& _stream, int _glyphBits, int _advanceBits, int _textVer)
    {
        SwfTextRecord* record = CORE_NEW SwfTextRecord();

        _stream.align();
        bool type = _stream.readFlag();
        // not really used should always be true
        if (type == true)
        {
            uint32_t reserved = _stream.readUInt(3);
				UNUSED(reserved);
            record->hasFont = _stream.readFlag();
            bool hasColour = _stream.readFlag();
            bool hasYOffset = _stream.readFlag();
            bool hasXOffset = _stream.readFlag();

            if (record->hasFont)
            {
                record->fontID = _stream.readUInt16();
            }
            if (hasColour)
            {
                record->colour = (_textVer >= 2) ? SwfRGBA::ReadRGBA(_stream) : SwfRGBA::ReadRGB(_stream);
            }
            int xoff = 0;
            int yoff = 0;
            if (hasXOffset)
            {
                xoff = _stream.readInt16();
            }
            if (hasYOffset)
            {
                yoff = _stream.readInt16();
            }
            record->offset = SwfVec2Twip(xoff, yoff);

            if (record->hasFont)
            {
                record->textHeight = _stream.readUInt16();
            }

            int glyphCount = _stream.readUInt8();
            record->glyphEntries = CORE_NEW_ARRAY SwfGlyphEntry[glyphCount];
				record->numGlyphEntries = glyphCount;
            for (int i = 0; i < glyphCount; ++i)
            {
					record->glyphEntries[i].index = _stream.readUInt(_glyphBits);
					record->glyphEntries[i].advance = _stream.readInt(_advanceBits);
            }
            return record;
        }
        return NULL;
    }
} /* Swf */
