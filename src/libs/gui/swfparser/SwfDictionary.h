// 
//  SwfDictionary.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_DICTIONARY_H)
#define _SWF_DICTIONARY_H

namespace Swf{
	// forward decl
	class SwfFont;
	class SwfCharacter;
	class SwfBitmap;
	
	struct SwfDictionary{
		typedef std::map<uint16_t, SwfFont*> FontMap;
		typedef std::map<uint16_t, SwfCharacter*> CharacterMap;
		typedef std::map<uint16_t, SwfBitmap*> BitmapMap;
		
		FontMap fonts;
		CharacterMap characters;
		BitmapMap bitmaps;
	};
}
#endif
