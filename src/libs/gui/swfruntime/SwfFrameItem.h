// 
//  SwfFrameItem.h
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFFRAMEITEM_H_
#define _SWFFRAMEITEM_H_

#include "actionscript/autogen/AsObject.h"

namespace Swf {
	// forward decl
	class SwfColourTransform;
	class SwfPlayer;
	class SwfMovieClip;
	
	enum FrameItemType {
		FIT_MOVIECLIP,
		FIT_DISPLAYOBJECT,
	};
	
	// ======================================================
	// = The base item in each frame list, the heart of Swf =
	// ======================================================
	class SwfFrameItem : public AutoGen::AsObject {
	public:
		typedef std::unordered_map<std::string, int> AsPropertyStringMap;
		
		virtual ~SwfFrameItem (){};
		virtual void Display(SwfPlayer* _player) = 0;
		virtual void Advance( float _elapsedMs ) {};
		virtual SwfMovieClip* GetAsMovieClip() { return NULL; }
		
		// numeric property handling
		virtual AutoGen::AsObjectHandle GetProperty( int _index );
		virtual void SetProperty( int _index, AutoGen::AsObjectHandle _val );
		
		virtual float GetXScale();
		virtual void SetXScale( float _xscale );
		virtual float GetYScale();
		virtual void SetYScale( float _yscale );
		virtual float GetRotation();
		virtual void SetRotation( float angle );
			
		virtual AutoGen::AsObjectHandle GetProperty( const std::string& _name );
		virtual void SetProperty( const std::string& _name, AutoGen::AsObjectHandle _handle );
		
	protected:
		SwfFrameItem (uint16_t _depth, FrameItemType _type, uint16_t _id, SwfMovieClip* _parent, const std::string& _name) :
			AutoGen::AsObject(AutoGen::APT_OBJECT) {
			depth = _depth;
			type = _type;
			id = _id;
			parent = _parent;
			name = _name;
			visible = true;
		}
		
	public:		
		uint16_t depth;
		FrameItemType type;
		uint16_t id;
		std::string name;
		Math::Matrix4x4 concatMatrix;
		SwfColourTransform* colourTransform;
		SwfMovieClip* parent;
		bool visible;
		static AsPropertyStringMap s_asPropertyStringMap;
	};
} /* Swf */


#endif /* _SWFFRAMEITEM_H_ */


