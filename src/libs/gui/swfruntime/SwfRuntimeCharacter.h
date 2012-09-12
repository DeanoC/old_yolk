// 
//  SwfCharacter.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWF_RUNTIME_CHARACTER_H_
#define _SWF_RUNTIME_CHARACTER_H_

namespace Swf {
	// forward decl
	class SwfPlayer;
	class SwfColourTransform;
	class SwfFrameItem;
	
	// ================================
	// = Runtime Character base class =
	// ================================
	class SwfRuntimeCharacter 
    {
	public:
        virtual void Display(	SwfPlayer* _player, 
								SwfFrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph) = 0;
	};
} /* Swf */


#endif /* _SWFCHARACTER_H_ */
