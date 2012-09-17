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
#include "scene/rendercontext.h"

namespace Swf {
	// forward decl
	class Player;
	class SwfColourTransform;
	class BasePath;
	// ==============================================================
	// = Base abstract class representing a Swf material fill style =
	// ==============================================================
	class FillStyle {
	public:
		FillStyle( Player* _player) : player(_player){}
		virtual ~FillStyle (){};
		
		virtual void apply( Scene::RenderContext* _ctx, const SwfColourTransform* _colourTransform, const BasePath* _path ) = 0;
	protected:
		Player* player;
	};
} /* Swf */


#endif /* YOLK_GUI_SWFRUNTIME_FILLSTYLE_H_ */

