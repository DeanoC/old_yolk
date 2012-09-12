// 
//  SwfActionByteCode.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFACTIONBYTECODE_H_
#define _SWFACTIONBYTECODE_H_

#include "SwfDisplayObject.h"
namespace Swf
{
	class SwfActionByteCode : public SwfControlElement
	{
	public:
		SwfActionByteCode() :
			SwfControlElement(CE_ACTIONBYTECODE),
			byteCode(0)
		{
		}

		uint8_t* byteCode;
		uint32_t lengthInBytes;
		bool 		isCaseSensitive; // Swf Pre 7 is case insenstive
	private:
		/* data */
	};
} /* Swf */


#endif /* _SWFACTIONBYTECODE_H_ */
