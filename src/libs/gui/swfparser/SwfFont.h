// 
//  SwfFont.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_FONT_H)
#define _SWF_FONT_H

//#include <inttypes.h>
#include <string>
namespace Swf
{
	// forward decl
	class SwfFont;
	class SwfShape;
	class SwfRect;
	class SwfStream;
	
    struct SwfKerningRecord
    {
        char kerningCode1;
        char kerningCode2;
        short kerningAdjustment;
	};
    /// <summary>
    /// On the Macintosh,
    /// _sans maps to Helvetica
    /// _serif maps to Times
    /// _typewriter maps to Courier
    ///
    ///On Windows,
    /// _sans maps to Arial
    /// _serif maps to Times New Roman
    /// _typewriter maps to Courier New
    /// </summary>
    class SwfFont
    {
	public:
		SwfFont() :
			id(0),
			flagHasLayout(false),
			flagShiftJIS(false),
			flagSmallText(false),
			flagANSI(false),
			flagItalic(false),
			flagBold(false),
			langCode(0),
			fontAscent(0),
			fontDescent(0),
			fontLeading(0),
			fontAdvanceTable(0),
			fontBoundsTable(0),
			fontKerningTable(0),
			shapes(0)
		{			
		}
		
		uint16_t id;
		bool flagHasLayout;
		bool flagShiftJIS;
		bool flagSmallText;
		bool flagANSI;
		bool flagItalic;
		bool flagBold;
		uint8_t langCode;
		std::map<uint16_t, uint32_t> codeTable;
		int fontAscent;
		int fontDescent;
		int fontLeading;
		uint16_t* fontAdvanceTable;
		SwfRect* fontBoundsTable;
		SwfKerningRecord* fontKerningTable;

		std::string name;
        
		int numGlyphs;
		SwfShape** shapes;

		static SwfFont* Read(SwfStream& _stream, int _fontVer);
	};	
}

#endif