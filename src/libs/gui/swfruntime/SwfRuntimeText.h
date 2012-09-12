#ifndef SWFRUNTIMETEXT_H_HI724XGZ
#define SWFRUNTIMETEXT_H_HI724XGZ

/*
 *  SwfRuntimeText.h
 *  Possessed
 *
 *  Created by Deano on 24/06/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "SwfRuntimeCharacter.h"

namespace Swf {
	// forward decl
	class SwfText;

	class SwfRuntimeText : public SwfRuntimeCharacter {
	public:
		SwfRuntimeText( SwfText* _text) 
			: text(_text) {}
			
		SwfText* text;

      virtual void Display(	SwfPlayer* _player, 
								SwfFrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph);
	protected:
		SwfRuntimeText() 
			: text(NULL) {}
	};
	
}
#endif /* end of include guard: SWFRUNTIMETEXT_H_HI724XGZ */
