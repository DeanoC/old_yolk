// 
//  SwfText.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFTEXT_H_
#define _SWFTEXT_H_

#include "SwfCharacter.h"
#include "SwfRGB.h"
namespace Swf
{
	// forward decl
	class SwfRect;
	class SwfMatrix;
	class SwfTextRecord;
	class SwfStream;
	
	enum SwfTextAlign
    {
        Left = 0,
        Right,
        Center,
        Justify
	};
	
	class SwfText : public SwfCharacter
	{
	public:
		SwfRect*  bounds;
		SwfMatrix* matrix;
		int			numRecords;
		SwfTextRecord** records;
		SwfText(uint16_t _id)
			: SwfCharacter(CT_TEXT, _id),
				bounds(NULL),
				matrix(NULL),
				records(NULL),
				numRecords(0)
		{
		}

		static SwfText* Read(SwfStream& _stream, int _textVer);
	};

	class SwfDynamicText : public SwfCharacter
	{
	public:
		SwfRect*  bounds;
		bool wordWrap;
		bool multiLine;
		bool password;
		bool readonlyText;
		bool autoSize;
		bool noSelect;
		bool hasBorder;
		bool isHtml;
		bool useOutlines;

		uint16_t fontId;
		int fontHeight;
		std::string fontClass;

		SwfRGBA fontColour;
		uint16_t maxTextLength;

		SwfTextAlign align;
		int leftMargin;
		int rightMargin;
		int indent;
		int leading;

		std::string varName;
		std::string initialText;

		static SwfDynamicText* Read(SwfStream& _stream);

		private:
		SwfDynamicText(uint16_t _id)
			: SwfCharacter(CT_DYNAMICTEXT, _id)
		{
		}
	};
} /* Swf */


#endif /* _SWFTEXT_H_ */
