// 
//  SwfShape.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfShape.h"
#include "SwfFillStyle.h"
#include "SwfLineStyle.h"
#include "SwfShapeRecord.h"
#include "SwfRect.h"
#include <vector>
namespace Swf
{
	SwfShape* SwfShape::Read(SwfStream& _stream, bool _withStyle, int _shapeVer) {
		_stream.align();
		SwfShape* shape = CORE_NEW SwfShape();
		if (_withStyle) {
			// fill style array
			uint16_t fillStyleCount = (uint16_t)_stream.readUInt8();
			if (fillStyleCount == 0xFF && (_shapeVer >= 2)) {
				fillStyleCount = _stream.readUInt16();
			}
			shape->fillStyleArray = CORE_NEW_ARRAY SwfFillStyle*[fillStyleCount];
			for (uint16_t i = 0; i < fillStyleCount; ++i) {
				shape->fillStyleArray[i] = SwfFillStyle::Read(_stream, (_shapeVer >= 3));
			}
			shape->numFillStyles = fillStyleCount;

			// line style array
			uint16_t lineStyleCount = (uint16_t)_stream.readUInt8();
			if (lineStyleCount == 0xFF) {
				lineStyleCount = _stream.readUInt16();
			}
			shape->lineStyleArray = CORE_NEW_ARRAY SwfLineStyle*[lineStyleCount];
			for (uint16_t i = 0; i < lineStyleCount; ++i) {
				shape->lineStyleArray[i] = SwfLineStyle::Read(_stream, _shapeVer);
			}
			shape->numLineStyles = lineStyleCount;
		}

		// bits
		uint16_t numFillBits = (uint16_t)_stream.readUInt(4);
		uint16_t numLineBits = (uint16_t)_stream.readUInt(4);
		// fill a list of shape records until we hit an end
		SwfShapeRecord* tmpRecord = NULL;

		std::vector<SwfShapeRecord*> tempShapeRecords;
		do {
			tmpRecord = SwfShapeRecord::Read(_stream, numFillBits, numLineBits, _shapeVer);
			tempShapeRecords.push_back( tmpRecord );
			if (tmpRecord->ShapeType() == SwfShapeRecord::StyleChange) {
				SwfStyleChangeShapeRecord* changer = (SwfStyleChangeShapeRecord*) tmpRecord;
				if (changer->newStylesFlag) {
					numFillBits = changer->numFillBits;
					numLineBits = changer->numLineBits;
				}
			}
		} while (tmpRecord->ShapeType() != SwfShapeRecord::End);

		shape->shapeRecords = CORE_NEW_ARRAY SwfShapeRecord*[tempShapeRecords.size()];
		for(unsigned int i=0;i < tempShapeRecords.size();++i) {
			shape->shapeRecords[i] = tempShapeRecords[i];
		}
		shape->numShapeRecords = tempShapeRecords.size();

		return shape;
	}
	
	SwfShapeObject* SwfShapeObject::Read(SwfStream& _stream, int _shapeVer) {
		uint16_t id = _stream.readUInt16();

		SwfShapeObject* obj = CORE_NEW SwfShapeObject(id);
		obj->bounds = SwfRect::Read(_stream);
		obj->shape = SwfShape::Read(_stream, true, _shapeVer);
		return obj;
	}
}