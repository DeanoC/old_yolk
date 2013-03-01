// 
//  SwfLineStyle.h
//  SwfParser
//  
//  Created by Deano on 2008-09-24.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFLINESTYLE_H_
#define _SWFLINESTYLE_H_

#include "SwfRGB.h"
namespace Swf
{
	// forward decl
	class SwfFillStyle;
	
	// ================
	// = SwfLineStyle =
	// ================	
	class SwfLineStyle
	{
	public:
		SwfLineStyle();
		
        enum CapStyle
        {
            CS_Round = 0,
            CS_None = 1,
            CS_Square = 2
		};
        enum JoinStyle
        {
            JS_Round = 0,
            JS_Bevel = 1,
            JS_Miter = 2
		};

        int width;
        CapStyle startCapStyle;
        CapStyle endCapStyle;
        JoinStyle joinStyle;
        
        bool hasFill;
        bool noHScale;
        bool noVScale;
        bool pixelHint;
        bool noClose;
        float miterLimitFactor;
        SwfRGBA colour;
        SwfFillStyle* fillStyle;

		static SwfLineStyle* Read(SwfStream& _stream, int _shapeVer);

	private:
		/* data */
	};
	
	class SwfMorphLineStyle : public SwfLineStyle
    {
	public:
        int endWidth;
        SwfRGBA endColour;
		static SwfMorphLineStyle* Read(SwfStream& _stream, int _morphVer);
	};
} /* Swf */


#endif /* _SWFLINESTYLE_H_ */
