#ifndef YOLK_GUI_SWFRUNTIME_DYNAMICTEXT_H_
#define YOLK_GUI_SWFRUNTIME_DYNAMICTEXT_H_

/*
 *  SwfRuntimeDynamicText.h
 *  Possessed
 *
 *  Created by Deano on 25/06/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui/SwfParser/SwfText.h"
#include "text.h"
namespace Swf {
	// forward decl
	class SwfDynamicText;
	
	class DynamicText : public Text {
	public:
		DynamicText ( SwfDynamicText* _text);
		
		virtual void display(	Player* _player, 
								Scene::RenderContext* _ctx,
								FrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph ) override;

		SwfDynamicText* dynText;
	private:
		void align_line( SwfTextAlign _align, SwfTextRecord* _rec, float x );
		void format_text(Player* _player, FrameItem* _parent);
	};
} /* Swf */ 

#endif /* end of include guard: YOLK_GUI_SWFRUNTIME_DYNAMICTEXT_H_ */
