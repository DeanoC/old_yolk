// 
//  SwfFrame.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFFRAME_H_
#define _SWFFRAME_H_

namespace Swf {
	// forward decl
	class SwfControlElement;
	
	class SwfFrame {
	public:
		virtual ~SwfFrame (){};
		
		typedef std::list<SwfControlElement*> FrameElementList;
		FrameElementList frameElements;
	private:
		/* data */
	};
} /* Swf */


#endif /* _SWFFRAME_H_ */

