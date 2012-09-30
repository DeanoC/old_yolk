#ifndef YOLK_GUI_SWFRUNTIME_FONT_H_
#define YOLK_GUI_SWFRUNTIME_FONT_H_

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
	class Shape;
	
	class Font {
	public:
		Font(const SwfFont* _font) 
			: font(_font) {};
						
		const SwfFont* font;
		std::vector<Shape*> glyphShapes;
	};
} /* Swf */ 



#endif /* end of include guard: YOLK_GUI_SWFRUNTIME_FONT_H_ */
