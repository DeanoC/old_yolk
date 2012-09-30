// 
//  SwfText.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfText.h"
#include "SwfRect.h"
#include "SwfMatrix.h"
#include "SwfTextRecord.h"
#include "SwfStream.h"
#include <list>
namespace Swf
{
	SwfText* SwfText::Read(SwfStream& _stream, int _textVer)
	{
		uint16_t id = _stream.readUInt16();
		SwfText* obj = CORE_NEW SwfText(id);
		obj->bounds = SwfRect::Read(_stream);
		obj->matrix = SwfMatrix::Read(_stream);

		int glyphBits = _stream.readUInt8();
		int advanceBits = _stream.readUInt8();

		std::list<SwfTextRecord*> tempTextRecords;

		SwfTextRecord* rec = NULL;
		do
		{
			rec = SwfTextRecord::Read(_stream, glyphBits, advanceBits, _textVer);
			if (rec != NULL)
			{
				tempTextRecords.push_back(rec);
			}
		} while (rec != NULL);
		obj->records = CORE_NEW_ARRAY SwfTextRecord*[tempTextRecords.size()];
		obj->numRecords = tempTextRecords.size();
		for( 	std::list<SwfTextRecord*>::iterator i = tempTextRecords.begin(); 
				i != tempTextRecords.end();
				++i )
		{
			obj->records[std::distance(tempTextRecords.begin(),i)] = *i;
		}

		return obj;
	}
	SwfDynamicText* SwfDynamicText::Read(SwfStream& _stream)
	{
		uint16_t id = _stream.readUInt16();
		SwfDynamicText* obj = CORE_NEW SwfDynamicText(id);
		obj->bounds = SwfRect::Read(_stream);
		_stream.align();
		bool hasText = _stream.readFlag();
		obj->wordWrap = _stream.readFlag();
		obj->multiLine = _stream.readFlag();
		obj->password = _stream.readFlag();
		obj->readonlyText = _stream.readFlag();
		bool hasColour = _stream.readFlag();
		bool hasMaxLength = _stream.readFlag();
		bool hasFont = _stream.readFlag();
		bool hasFontClass = _stream.readFlag();
		obj->autoSize = _stream.readFlag();
		bool hasLayout = _stream.readFlag();
		obj->noSelect = _stream.readFlag();
		obj->hasBorder = _stream.readFlag();
		bool wasStatic = _stream.readFlag();
		UNUSED(wasStatic);
		obj->isHtml = _stream.readFlag();
		obj->useOutlines = _stream.readFlag();

		if (hasFont)
		{
		    obj->fontId = _stream.readUInt16();
		}
		if (hasFontClass)
		{
		    obj->fontClass = _stream.readString();
		}
		if (hasFont)
		{
		    obj->fontHeight = _stream.readUInt16();
		}
		if (hasColour)
		{
		    obj->fontColour = SwfRGBA::ReadRGBA(_stream);
		}
		else
		{
		    obj->fontColour = SwfRGBA(0.0f, 0.0f, 0.0f, 1.0f);
		}
		if (hasMaxLength)
		{
		    obj->maxTextLength = _stream.readUInt16();
		}
		else
		{
		    obj->maxTextLength = 1024; // unlimited technically needs more??
		}
		if (hasLayout)
		{
		    obj->align = (SwfTextAlign) _stream.readUInt8();
		    obj->leftMargin = _stream.readUInt16();
		    obj->rightMargin = _stream.readUInt16();
		    obj->indent = _stream.readUInt16();
		    obj->leading = _stream.readInt16();
		}
		obj->varName = _stream.readString();
		if (hasText)
		{
		    obj->initialText = _stream.readString();
		}

		return obj;
	}
    
} /* Swf */
