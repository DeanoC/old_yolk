// 
//  SwfShapeRecord.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfShapeRecord.h"
#include "SwfStream.h"
#include "SwfLineStyle.h"
#include "SwfFillStyle.h"
namespace Swf
{
	SwfShapeRecord* SwfShapeRecord::Read(SwfStream& _stream, int _fillbits, int _linebits, int _shapeVer)
	{
		bool typeFlag = _stream.readFlag();
		if (typeFlag == false)
		{
			// is a style change or end flag
			bool newStylesFlag = _stream.readFlag();
			bool lineStyleFlag = _stream.readFlag();
			bool fillStyle1Flag = _stream.readFlag();
			bool fillStyle0Flag = _stream.readFlag();
			bool moveToFlag = _stream.readFlag();
			if ((newStylesFlag || lineStyleFlag || fillStyle0Flag || fillStyle1Flag || moveToFlag) == false)
			{
				// all set to 0, so this is an EndShapeRecord
				return CORE_NEW SwfEndShapeRecord();
			}

			SwfStyleChangeShapeRecord* changer = CORE_NEW SwfStyleChangeShapeRecord();
			if (moveToFlag)
			{
				int moveBits = (int)_stream.readUInt(5);
				int x = _stream.readInt(moveBits);
				int y = _stream.readInt(moveBits);
				changer->moveDelta = SwfVec2Twip(x, y);
				changer->moveToFlag = true;
			}
			if (fillStyle0Flag)
			{
				changer->fillStyle0 = (uint16_t)_stream.readUInt(_fillbits);
				changer->fillStyle0Flag = true;
			}
			if (fillStyle1Flag)
			{
				changer->fillStyle1 = (uint16_t)_stream.readUInt(_fillbits);
				changer->fillStyle1Flag = true;
			}
			if (lineStyleFlag)
			{
				changer->lineStyle = (uint16_t)_stream.readUInt(_linebits);
				changer->lineStyleFlag = true;
			}
			if (newStylesFlag)
			{
				uint16_t fillStyleCount = (uint16_t) _stream.readUInt8();
				if (fillStyleCount == 0xFF && (_shapeVer >= 2) )
				{
				    fillStyleCount = _stream.readUInt16();
				}
				changer->fillStyleArray = CORE_NEW_ARRAY SwfFillStyle*[fillStyleCount];
				for (uint16_t i = 0; i < fillStyleCount; ++i)
				{
				    changer->fillStyleArray[i] = SwfFillStyle::Read(_stream, (_shapeVer >= 3));
				}
				changer->numFillStyles = fillStyleCount;
				uint16_t lineStyleCount = (uint16_t)_stream.readUInt8();
				if (lineStyleCount == 0xFF)
				{
				    lineStyleCount = _stream.readUInt16();
				}
				changer->lineStyleArray = CORE_NEW_ARRAY SwfLineStyle*[lineStyleCount];
				for (uint16_t i = 0; i < lineStyleCount; ++i)
				{
				    changer->lineStyleArray[i] = SwfLineStyle::Read(_stream, _shapeVer);
				}
				changer->numLineStyles = lineStyleCount;
				changer->numFillBits = (uint16_t)_stream.readUInt(4);
				changer->numLineBits = (uint16_t)_stream.readUInt(4);
				changer->newStylesFlag = true;
			}
			return changer;
		}
		else
		{
			// its an edge
			// now straight or curved?
			bool isStraight = _stream.readFlag();
			if (isStraight)
			{
				return SwfStraightEdgeShapeRecord::Read(_stream);
			}
			else
			{
				return SwfCurvedEdgeShapeRecord::Read(_stream);
			}
		}
	}
} /* Swf */
