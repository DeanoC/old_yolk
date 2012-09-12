// 
//  SwfRFS_GLTnL_iPhone.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFRFS_GLTNL_IPHONE_H_
#define _SWFRFS_GLTNL_IPHONE_H_

#if PLATFORM == iPhone

#include "SwfRuntimeFillStyle.h"
#include "gui/SwfParser/SwfRGB.h"

namespace Swf {
	// forward decl
	class SwfPlayer;
	class SwfGradientFillStyle;
	class SwfGradientTexture;
	class SwfBitmapFillStyle;
	class SwfRuntimeBitmap;
	class SwfLineStyle;
	
	namespace iPhone {
		class SwfRFSSolidColour : public SwfRuntimeFillStyle {
		public:
	        SwfRFSSolidColour( SwfPlayer* _player, const SwfRGBA& _col) :
				SwfRuntimeFillStyle(_player) {
	            colour = _col;
	        }
			virtual APPLY_RESULT TestApply(const SwfColourTransform* _colourTransform);
			virtual APPLY_RESULT Apply(const SwfColourTransform* _colourTransform);
			
	        SwfRGBA colour;
		private:
			/* data */
		};
		
	    class SwfRFSGradient : public SwfRuntimeFillStyle {
		public:
			SwfRFSGradient(SwfPlayer* _player, SwfGradientFillStyle* _gradientFill);
			virtual APPLY_RESULT TestApply(const SwfColourTransform* _colourTransform);
			virtual APPLY_RESULT Apply(const SwfColourTransform* _colourTransform);
			
	        SwfGradientTexture* gradTex;
	        Math::Matrix4x4 matrix;
		};		
		
		class SwfRFSTexture : public SwfRuntimeFillStyle {
		public:
			SwfRFSTexture(SwfPlayer* _player, SwfBitmapFillStyle* _bitmapFill);
			
			virtual APPLY_RESULT TestApply(const SwfColourTransform* _colourTransform);
			virtual APPLY_RESULT Apply(const SwfColourTransform* _colourTransform);
			
			SwfRuntimeBitmap* bitmapTex;
			Math::Matrix4x4 matrix;	
		};
		
		class SwfRFSLine : public SwfRuntimeFillStyle {
		public:
			SwfRFSLine(SwfPlayer* _player, SwfLineStyle* _lineFill);

			virtual APPLY_RESULT TestApply(const SwfColourTransform* _colourTransform);
			virtual APPLY_RESULT Apply(const SwfColourTransform* _colourTransform);

		private:
			SwfRGBA colour;
			Math::Vector2 lineWidth;
		};
	}	
} /* Swf */

#endif // PLATFORM == IPHONE

#endif /* _SWFRFS_GLTNL_IPHONE_H_ */
