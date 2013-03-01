/*
 *  SwfFillStyle.h
 *  SkyGlow-Iphone
 *
 *  Created by Deano on 24/09/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _SWFFILLSTYLE_H_
#define _SWFFILLSTYLE_H_

#include "SwfRGB.h"

namespace Swf
{
	//forward decl
	class SwfStream;
	class SwfMatrix;
	class SwfGradient;
	
    // the spec defines Morph Fill Types as different from non morph fills but theres no real
    // reason for this, so instead i've banged them together in one type. In the stream
    // the type of the Morph Fill Types are the same (ie. Solid Fill = 0x00 for both)
    // on load I give them a different number (enum + 0x100) to mark morphs fills
    enum FillType
    {
        SolidFill                           = 0x00,
        LinearGradientFill                  = 0x10,
        RadialGradientFill                  = 0x12,
        FocalRadialGradientFill             = 0x13,
        RepeatingBitmapFill                 = 0x40,
        ClippedBitmapFill                   = 0x41,
        NonSmoothedRepeatingBitmapFill      = 0x42,
        NonSmoothedClippedBitmapFill        = 0x43,

        MorphSolidFill                      = 0x100,
        MorphLinearGradientFill             = 0x110,
        MorphRadialGradientFill             = 0x112,
        MorphFocalRadialGradientFill        = 0x113,
        MorphRepeatingBitmapFill            = 0x140,
        MorphClippedBitmapFill              = 0x141,
        MorphNonSmoothedRepeatingBitmapFill = 0x142,
        MorphNonSmoothedClippedBitmapFill   = 0x143,
	};
	
	class SwfFillStyle
	{
	public:
	    SwfFillStyle (FillType _type)
	    {
            fillType = _type;
	    }
		SwfFillStyle(){}
	
        FillType fillType;

		static SwfFillStyle* Read(SwfStream& _stream, bool _shape3orMore);
		static SwfFillStyle* ReadMorph(SwfStream& _stream);
	private:
		
	};
	
    class SwfSolidFillStyle : public SwfFillStyle
    {
	public:
        SwfSolidFillStyle(SwfRGBA _colour) :
            SwfFillStyle(SolidFill)
        {
            colour = _colour;
        }

        SwfRGBA colour;
	};

    class SwfMorphSolidFillStyle : public SwfFillStyle
    {
	public:
        SwfMorphSolidFillStyle(SwfRGBA _startColour, SwfRGBA _endColour) :
            SwfFillStyle(MorphSolidFill)
        {
            startColour = _startColour;
            endColour = _endColour;
        }

        SwfRGBA startColour;
        SwfRGBA endColour;
	};


    class SwfGradientFillStyle : public SwfFillStyle
    {
	public:
        SwfGradientFillStyle(FillType _type, SwfMatrix* _matrix, SwfGradient* _gradient) :
            SwfFillStyle(_type)
        {
            matrix = _matrix;
            gradient = _gradient;
        }

        SwfMatrix* Matrix()
        {
			return matrix;
        }
		SwfGradient* Gradient() 
        { 
			return gradient;
        }
    private:
        SwfMatrix* matrix;
		SwfGradient* gradient;
	};

    class SwfFocalGradientFillStyle : public SwfGradientFillStyle
    {
	public:
        SwfFocalGradientFillStyle(FillType _type, SwfMatrix* _matrix, SwfGradient* _gradient, float _focalPt) :
            SwfGradientFillStyle(_type, _matrix, _gradient)
        {
            focalPt = _focalPt;
        }

        float FocalPt()
        {
			return focalPt;
        }

	private:
	 	float focalPt;
	};
    
	class SwfMorphGradientFillStyle : public SwfGradientFillStyle
    {
	public:
        SwfMorphGradientFillStyle(FillType _type, SwfMatrix* _startMatrix, SwfGradient* _startGradient, SwfMatrix* _endMatrix, SwfGradient* _endGradient) :
            SwfGradientFillStyle(_type, _startMatrix, _startGradient)
        {
            endMatrix = _endMatrix;
            endGradient = _endGradient;
        }

        SwfMatrix* EndMatrix()
        {
			return endMatrix;
        }
        SwfGradient* EndGradient()
        {
			return endGradient;
        }
	private:
        SwfMatrix* endMatrix;
		SwfGradient* endGradient;
	};
	
    class SwfBitmapFillStyle : public SwfFillStyle
    {
	public:
        SwfBitmapFillStyle(FillType _type, uint16_t _bitmapID, SwfMatrix* _matrix) :
            SwfFillStyle(_type)
        {
            bitmapID = _bitmapID;
            matrix = _matrix;
        }

        uint16_t BitmapID()
        {
			return bitmapID;
        }
        SwfMatrix* Matrix()
        {
			return matrix;
        }
	private:
        uint16_t bitmapID;
		SwfMatrix* matrix;
	};

    class SwfMorphBitmapFillStyle : public SwfBitmapFillStyle
    {
    public:
		SwfMorphBitmapFillStyle(FillType _type, uint16_t _bitmapID, SwfMatrix* _startMatrix, SwfMatrix* _endMatrix) :
            SwfBitmapFillStyle(_type, _bitmapID, _startMatrix)
        {
            endMatrix = _endMatrix;
        }

        SwfMatrix* EndMatrix()
        {
			return endMatrix; 
        }
	private:
        SwfMatrix* endMatrix;
	};
	
} /* Swf */



#endif /* _SWFFILLSTYLE_H_ */

