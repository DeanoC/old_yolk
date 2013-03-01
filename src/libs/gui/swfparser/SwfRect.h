// 
//  SwfRect.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_RECT_H)
#define _SWF_RECT_H

namespace Swf {
	// forward decl
	class SwfStream;
	
	class SwfRect{
	public:
		int minX;
		int minY;
		int maxX;
		int maxY;
		
		SwfRect(){
			minX = maxX = 0;
			minY = maxY = 0;
		}
		
		SwfRect(int _minx, int _miny, int _maxx, int _maxy){
			minX = _minx;
			minY = _miny;
			maxX = _maxx;
			maxY = _maxy;
		}
		
		int Width() {
			return maxX - minX;
		}
		int Height() {
			return maxY - minY;
		}
		
		static SwfRect* Read(SwfStream& _stream);
			
	};
}

#endif