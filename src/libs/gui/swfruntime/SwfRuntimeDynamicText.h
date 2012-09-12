#ifndef SWFRUNTIMEDYNAMICTEXT_H_N10D57N5
#define SWFRUNTIMEDYNAMICTEXT_H_N10D57N5

/*
 *  SwfRuntimeDynamicText.h
 *  Possessed
 *
 *  Created by Deano on 25/06/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "gui/SwfParser/SwfText.h"
#include "SwfRuntimeText.h"
namespace Swf {
	// forward decl
	class SwfDynamicText;
	
	class SwfRuntimeDynamicText : public SwfRuntimeText {
	public:
		SwfRuntimeDynamicText ( SwfDynamicText* _text);
		
	   virtual void Display(	SwfPlayer* _player, 
								SwfFrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph);

		SwfDynamicText* dynText;
	private:
		void align_line( SwfTextAlign _align, SwfTextRecord* _rec, float x );
		void format_text(SwfPlayer* _player, SwfFrameItem* _parent);
	};
} /* Swf */ 

#endif /* end of include guard: SWFRUNTIMEDYNAMICTEXT_H_N10D57N5 */
