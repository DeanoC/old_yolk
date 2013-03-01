#ifndef YOLK_GUI_SWFRUNTIME_TEXT_H_
#define YOLK_GUI_SWFRUNTIME_TEXT_H_

/*
 *  SwfRuntimeText.h
 *  Possessed
 *
 *  Created by Deano on 24/06/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "character.h"

namespace Swf {
	// forward decl
	class SwfText;

	class Text : public Character {
	public:
		Text( SwfText* _text) : text(_text) {}
			
		SwfText* text;

		virtual void display(	Player* _player,
								Scene::RenderContext* _ctx,
								FrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph) override;
	protected:
		Text() : text(NULL) {}
	};
	
}
#endif /* end of include guard: YOLK_GUI_SWFRUNTIME_TEXT_H_ */
