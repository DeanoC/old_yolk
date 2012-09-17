// 
//  SwfRuntimeBitmap.cpp
//  deano
//  
//  Created by Deano on 2008-10-13.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/SwfBitmap.h"
#include "gui/SwfParser/SwfImage.h"
#include "utils.h"
#include "bitmap.h"

namespace Swf {
	Bitmap::Bitmap( const SwfBitmap* _bitmap ) {
		bitmap = _bitmap;
		
/*		CALL_GL(glGenTextures(1, &texNum));
		int width = Core::NextPow2(_bitmap->width);
		int height = Core::NextPow2(_bitmap->height);
		uint32_t *tmp = CORE_NEW_ARRAY uint32_t[height * width];
		memset(tmp,0xFF,width*height*sizeof(uint32_t));
		
        for(int y =0;y < _bitmap->height;++y)
            for(int x=0;x < _bitmap->width;++x)
            {
                tmp[(y * width) + x] = _bitmap->image->ToPackedABGR(x,y);
            }
		
		CALL_GL(glBindTexture( GL_TEXTURE_2D, texNum ));
		CALL_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp));
		CALL_GL(glBindTexture( GL_TEXTURE_2D, 0 ));
		CORE_DELETE_ARRAY tmp;
		
		offset.x = 0; offset.y = 0;
		scale.x = (float)_bitmap->width / (float) width;
		scale.y = (float)_bitmap->height / (float) height;*/
		TODO_ASSERT( false && "GL code" );
	}
} /* Swf */ 


