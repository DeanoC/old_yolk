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
#include "scene/texture.h"
#include "core/core_utils.h"
#include "utils.h"
#include "bitmap.h"

namespace Swf {
	Bitmap::Bitmap( const SwfBitmap* _bitmap ) {
		bitmap = _bitmap;
		
		int width = Core::getNextPow2(_bitmap->width );
		int height = Core::getNextPow2(_bitmap->height );
		uint32_t *tmpTextureStorage = CORE_NEW_ARRAY uint32_t[ height * width ];
		memset(tmpTextureStorage,0xFF,width*height*sizeof(uint32_t));
		
        for(int y =0;y < _bitmap->height;++y) {
            for(int x=0;x < _bitmap->width;++x) {
                tmpTextureStorage[(y * width) + x] = _bitmap->image->ToPackedABGR(x,y);
            }
		}
	
		using namespace Scene;

		// TODO mipmaps
		Texture::CreationInfo ccs = Texture::TextureCtor( 
			RCF_TEX_2D | RCF_PRG_READ | RCF_ACE_IMMUTABLE, GTF_SRGB8_ALPHA8, 
			width, height, 1, 1, 1, 1, 
			tmpTextureStorage, width * sizeof(uint32_t)  );
		textureHandle.reset( TextureHandle::create( "_SWF_Bitmapex", &ccs, Core::RMRF_DONTCACHE ) );
		textureHandle.acquire(); // force acquire so we can free the tmp storage buffer

		CORE_DELETE_ARRAY tmpTextureStorage;
		
		offset.x = 0; offset.y = 0;
		scale.x = (float)_bitmap->width / (float) width;
		scale.y = (float)_bitmap->height / (float) height;
	}

} /* Swf */ 


