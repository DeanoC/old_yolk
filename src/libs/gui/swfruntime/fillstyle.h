// 
//  SwfRuntimeFillStyle.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_
#define YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_
#pragma once

#include "core/vector_math.h"

namespace Swf {
	// forward decl
	class Player;
	class SwfColourTransform;
	// ==============================================================
	// = Base abstract class representing a Swf material fill style =
	// ==============================================================
	class FillStyle {
	public:
		enum APPLY_RESULT {
			NO_OUTPUT,
			SOLID_OUTPUT,
			BLEND_OUTPUT,
		};
		FillStyle( Player* _player) : player(_player){}
		virtual ~FillStyle (){};
		
		virtual APPLY_RESULT testApply( const SwfColourTransform* _colourTransform ) = 0;
		virtual APPLY_RESULT apply( const SwfColourTransform* _colourTransform ) = 0;
	protected:
		Player* player;
	};
} /* Swf */


#endif /* YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_ */

