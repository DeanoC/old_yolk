// 
//  SwfFont.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfFont.h"
#include "SwfStream.h"
#include "SwfRect.h"
#include "SwfShape.h"
namespace Swf
{
	SwfFont* SwfFont::Read(SwfStream& _stream, int _fontVer)
	{
		SwfFont* font = CORE_NEW SwfFont();
		_stream.align();

		uint32_t* offsetTable = 0;
		uint32_t codeTableOffset = 0;

		font->id = _stream.readUInt16();

		if (_fontVer == 1)
		{
			uint64_t marker1 = _stream.marker();

			uint32_t offset0 = (uint32_t)_stream.readUInt16();
			uint16_t numGlyphs1 = (uint16_t)(offset0 / 2);
			font->numGlyphs = numGlyphs1;

			offsetTable = CORE_NEW_ARRAY uint32_t[numGlyphs1];
			offsetTable[0] = offset0;
			for (int i = 1; i < numGlyphs1; ++i)
			{
				offsetTable[i] = (uint32_t)_stream.readUInt16();
			}

			assert( _stream.marker() == (marker1 + offsetTable[0]));

			font->shapes = CORE_NEW_ARRAY SwfShape*[numGlyphs1];
			// shape records here
			for (int i = 0; i < numGlyphs1; ++i)
			{
				uint64_t offset = marker1 + offsetTable[i];
				_stream.setToMarker(offset);
				font->shapes[i] = SwfShape::Read(_stream, false, _fontVer);
			}
			return font;
		}
		font->flagHasLayout = _stream.readFlag();
		font->flagShiftJIS = _stream.readFlag();
		font->flagSmallText = _stream.readFlag();
		font->flagANSI = _stream.readFlag();
		bool fontFlagWideOffsets = _stream.readFlag();
		bool fontFlagWideCodes = _stream.readFlag();
		font->flagItalic = _stream.readFlag();
		font->flagBold = _stream.readFlag();
		font->langCode = _stream.readUInt8();

		font->name = _stream.readLengthString();

		uint16_t numGlyphs = _stream.readUInt16();
		font->numGlyphs = numGlyphs;

		uint64_t marker = _stream.marker();

		offsetTable = CORE_NEW uint32_t[numGlyphs];
		if (fontFlagWideOffsets) {
			for (int i = 0; i < numGlyphs; ++i) {
				offsetTable[i] = _stream.readUInt32();
			}
			codeTableOffset = _stream.readUInt32();
		} else {
			for (int i = 0; i < numGlyphs; ++i) {
				offsetTable[i] = (uint32_t)_stream.readUInt16();
			}
			codeTableOffset = _stream.readUInt16();
		}

		font->shapes = CORE_NEW SwfShape*[numGlyphs];
		// shape records here
		for (int i = 0; i < numGlyphs; ++i) {
			uint64_t offset = marker + offsetTable[i];
			_stream.setToMarker(offset);
			font->shapes[i] = SwfShape::Read(_stream, false, _fontVer);
		}

		for (int i = 0; i < numGlyphs; ++i) {
			if (fontFlagWideCodes) {
				int code = _stream.readUInt16();
				font->codeTable[(uint16_t)code] = i;
			} else {
				int code = _stream.readUInt8();
				font->codeTable[(uint16_t)code] = i;
			}
		}

		if (font->flagHasLayout) {
			font->fontAscent = _stream.readInt16();
			font->fontDescent = _stream.readInt16();
			font->fontLeading = _stream.readInt16();
			font->fontAdvanceTable = CORE_NEW_ARRAY uint16_t[numGlyphs];
			for (int i = 0; i < numGlyphs; ++i) {
				font->fontAdvanceTable[i] = _stream.readInt16();
			}
			font->fontBoundsTable = CORE_NEW_ARRAY SwfRect[numGlyphs];
			for (int i = 0; i < numGlyphs; ++i) {
				font->fontBoundsTable[i] = *SwfRect::Read(_stream);
			}
			int kerningCount = _stream.readUInt16();
			font->fontKerningTable = CORE_NEW_ARRAY SwfKerningRecord[kerningCount];
			for (int i = 0; i < kerningCount; ++i) {
				if (fontFlagWideCodes) {
					font->fontKerningTable[i].kerningCode1 = (char)_stream.readUInt16();
					font->fontKerningTable[i].kerningCode2 = (char)_stream.readUInt16();
				} else {
					font->fontKerningTable[i].kerningCode1 = (char)_stream.readUInt8();
					font->fontKerningTable[i].kerningCode2 = (char)_stream.readUInt8();
				}
				font->fontKerningTable[i].kerningAdjustment = _stream.readInt16();
			}
		}
		return font;
	}
}