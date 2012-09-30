// 
//  SwfSprite.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFSPRITE_H_
#define _SWFSPRITE_H_
#include "SwfCharacter.h"
namespace Swf
{
	// forward decl
	class SwfFrame;
	
	class SwfSprite : public SwfCharacter
	{
	public:
		SwfSprite(uint16_t _id)
			: SwfCharacter(CT_SPRITE, _id)
		{
		}

		uint16_t frameCount;

		std::vector<SwfFrame*> frames;
	private:
		/* data */
	};
} /* Swf */


#endif /* _SWFSPRITE_H_ */
