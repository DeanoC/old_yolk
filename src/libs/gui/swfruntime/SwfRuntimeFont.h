#ifndef SWFRUNTIMEFONT_H_TBGWN12T
#define SWFRUNTIMEFONT_H_TBGWN12T

/*
 *  SwfRuntimeFont.h
 *  Possessed
 *
 *  Created by Deano on 24/06/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

namespace Swf {
	// forward decl
	class SwfFont;
	class SwfRuntimeShape;
	
	class SwfRuntimeFont {
	public:
		SwfRuntimeFont(const SwfFont* _font) 
			: font(_font) {};
						
		const SwfFont* font;
		std::vector<SwfRuntimeShape*> glyphShapes;
	};
} /* Swf */ 



#endif /* end of include guard: SWFRUNTIMEFONT_H_TBGWN12T */
