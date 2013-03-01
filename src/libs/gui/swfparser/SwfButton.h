/*
 *  SwfButton.h
 *  SwfPreview
 *
 *  Created by Deano on 13/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef SWFBUTTON_H_WL9YSR3A
#define SWFBUTTON_H_WL9YSR3A

#include "SwfStream.h"
#include "SwfCharacter.h"

namespace Swf
{
	class SwfButtonRecord {
		
	};
	
	class SwfButtonObject : public SwfCharacter
	{
	public:
		static SwfButtonObject* Read( SwfStream& _stream, int _length, int _version );

	private:
		SwfButtonObject(uint16_t _id) : 	
			SwfCharacter(CT_BUTTON, _id)
		{}
	};
	
} /* Swf */ 


#endif /* end of include guard: SWFBUTTON_H_WL9YSR3A */

