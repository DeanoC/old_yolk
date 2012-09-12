// 
//  SwfRuntimeFillStyle.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFRUNTIMEFILLSTYLE_H_
#define _SWFRUNTIMEFILLSTYLE_H_

#include "core/vector_math.h"

namespace Swf {
	// forward decl
	class SwfPlayer;
	class SwfColourTransform;
	// ==============================================================
	// = Base abstract class representing a Swf material fill style =
	// ==============================================================
	class SwfRuntimeFillStyle {
	public:
		enum APPLY_RESULT {
			NO_OUTPUT,
			SOLID_OUTPUT,
			BLEND_OUTPUT,
		};
		SwfRuntimeFillStyle( SwfPlayer* _player) : player(_player){}
		virtual ~SwfRuntimeFillStyle (){};
		
		virtual APPLY_RESULT TestApply(const SwfColourTransform* _colourTransform) = 0;
		virtual APPLY_RESULT Apply(const SwfColourTransform* _colourTransform) = 0;
	protected:
		SwfPlayer* player;
	};
} /* Swf */


#endif /* _SWFRUNTIMEFILLSTYLE_H_ */

