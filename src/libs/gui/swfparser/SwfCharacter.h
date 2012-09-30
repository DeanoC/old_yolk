// 
//  SwfCharacter.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_CHARACTER_H)
#define _SWF_CHARACTER_H

namespace Swf
{
	enum CharacterTypes {
		CT_SHAPEOBJECT,
		CT_SPRITE,
		CT_TEXT,
		CT_DYNAMICTEXT,
		CT_MORPHSHAPE,
		CT_BITMAP,
		CT_BUTTON
	};
	/// <summary>
	/// shapes and sprites both share an ID space called Character ID this is what the id links to
	/// </summary>
	class SwfCharacter
	{
	public:
		CharacterTypes type;
		uint16_t id;
	protected:
		SwfCharacter(CharacterTypes _type, uint16_t _id)
		{
			type = _type;
			id = _id;
		}
	};
}

#endif