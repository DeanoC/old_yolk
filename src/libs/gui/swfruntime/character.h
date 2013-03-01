// 
//  SwfCharacter.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_CHARACTER_H_
#define YOLK_GUI_SWFRUNTIME_CHARACTER_H_

namespace Scene {
	class RenderContext;
};
namespace Swf {
	// forward decl
	class SwfColourTransform;
	class Player;
	class FrameItem;
	
	// ================================
	// = Runtime Character base class =
	// ================================
	class Character {
	public:
        virtual void display(	Player* _player,
								Scene::RenderContext* _ctx, 
								FrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph) = 0;
	};
} /* Swf */


#endif /* _SWFCHARACTER_H_ */
