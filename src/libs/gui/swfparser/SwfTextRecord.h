// 
//  SwfTextRecord.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFTEXTRECORD_H_
#define _SWFTEXTRECORD_H_

#include "SwfRGB.h"
#include "SwfVec2Twip.h"

namespace Swf
{
	// forward decl
	class SwfStream;

	struct SwfGlyphEntry
	{
		uint32_t index;
		int advance;
	};

	class SwfTextRecord
	{
	public:
		virtual ~SwfTextRecord (){};

		bool hasFont;
		uint16_t fontID;
		SwfRGBA colour;
		SwfVec2Twip offset;
		int textHeight;

		SwfGlyphEntry* glyphEntries;
		int numGlyphEntries;
		static SwfTextRecord* Read(SwfStream& _stream, int _glyphBits, int _advanceBits, int _textVer);

	private:
		/* data */
	};
} /* Swf */


#endif /* _SWFTEXTRECORD_H_ */

