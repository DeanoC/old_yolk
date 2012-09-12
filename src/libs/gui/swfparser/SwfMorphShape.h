// 
//  SwfMorphShape.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFMORPHSHAPE_H_
#define _SWFMORPHSHAPE_H_

#include "SwfCharacter.h"
namespace Swf
{
	// forward decl
	class SwfFillStyle;
	class SwfLineStyle;
	class SwfShape;
	class SwfStream;
	class SwfRect;
	
	class SwfMorphShape : public SwfCharacter
    {
	public:
        SwfRect* startRect;
        SwfRect* endRect;
        
        SwfFillStyle** fillStyleArray; // note: can be null
        SwfLineStyle** lineStyleArray; // note: can be null
        SwfShape* startEdges;
        SwfShape* endEdges;

		static SwfMorphShape* Read(SwfStream& _stream, int _morphVer);

	private:
        SwfMorphShape(uint16_t _id) :
            SwfCharacter(CT_MORPHSHAPE, _id)
        {
        }
	};
} /* Swf */


#endif /* _SWFMORPHSHAPE_H_ */
