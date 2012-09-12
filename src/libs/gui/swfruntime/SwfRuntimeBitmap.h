// 
//  SwfRuntimeBitmap.h
//  deano
//  
//  Created by Deano on 2008-10-13.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef SWFRUNTIMEBITMAP_H_QWW3E5EC
#define SWFRUNTIMEBITMAP_H_QWW3E5EC

namespace Swf {
	// forward decl
	class SwfBitmap;
	
	class SwfRuntimeBitmap {
	public:
		SwfRuntimeBitmap(const SwfBitmap* _bitmap);
				
		unsigned int texNum;
		Math::Vector2 offset;
		Math::Vector2 scale;
		
		const SwfBitmap* bitmap;
	};
} /* Swf */ 

#endif /* end of include guard: SWFRUNTIMEBITMAP_H_QWW3E5EC */

