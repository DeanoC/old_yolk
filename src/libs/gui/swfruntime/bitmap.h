// 
//  SwfRuntimeBitmap.h
//  deano
//  
//  Created by Deano on 2008-10-13.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_BITMAP_H_
#define YOLK_GUI_SWFRUNTIME_BITMAP_H_

namespace Swf {
	// forward decl
	class SwfBitmap;
	
	class Bitmap {
	public:
		Bitmap( const SwfBitmap* _bitmap );
				
		Core::ScopedResourceHandle<Scene::TextureHandle>	textureHandle;

		Math::Vector2 offset;
		Math::Vector2 scale;
		
		const SwfBitmap* bitmap;
	};
} /* Swf */ 

#endif /* end of include guard: YOLK_GUI_SWFRUNTIME_BITMAP_H_ */

