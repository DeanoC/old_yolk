// 
//  SwfShape.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_SHAPE_H)
#define _SWF_SHAPE_H

#include "SwfShapeRecord.h"
#include "SwfCharacter.h"
namespace Swf {
	// forward decl
	class SwfFillStyle;
	class SwfLineStyle;
	class SwfRect;
	
	class SwfShape
    {
	public:
		SwfShape() :
			numFillStyles(0),
			numLineStyles(0),
			numShapeRecords(0),
			fillStyleArray(0),
			lineStyleArray(0),
			shapeRecords(0)
		{
			
		}
		int numFillStyles;
		int numLineStyles;
		int numShapeRecords;
		
        SwfFillStyle** fillStyleArray; // note: can be null
        SwfLineStyle** lineStyleArray; // note: can be null
        SwfShapeRecord** shapeRecords;

		static SwfShape* Read(SwfStream& _stream, bool _withStyle, int _shapeVer);
	};
	
	class SwfShapeObject : public SwfCharacter
	{
	public:
		SwfRect*  bounds;
		SwfShape* shape;
		
		static SwfShapeObject* Read(SwfStream& _stream, int _shapeVer);
	private:
		SwfShapeObject(uint16_t _id)
		    : 	SwfCharacter(CT_SHAPEOBJECT, _id),
				bounds(0),
				shape(0)
		{
		}
	};
}
#endif