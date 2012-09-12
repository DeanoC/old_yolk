// 
//  SwfBitmap.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFBITMAP_H_
#define _SWFBITMAP_H_

#include "SwfCharacter.h"

namespace Swf
{
	//forward decl
	class SwfImage;
	
	class SwfBitmap : public SwfCharacter
    {
    public:
		SwfBitmap(uint16_t _id)
            : SwfCharacter(CT_BITMAP, _id)
        {
        }

        int width;
        int height;

        SwfImage* image;
	};
} /* Swf */


#endif /* _SWFBITMAP_H_ */


