// 
//  SwfFrameItem.h
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_FRAMEITEM_H_
#define YOLK_GUI_SWFRUNTIME_FRAMEITEM_H_

#include "actionscript/AsObject.h"

namespace Scene {
	class RenderContext;
};

namespace Swf {
	// forward decl
	class SwfColourTransform;
	class Player;
	class MovieClip;
	
	enum FrameItemType {
		FIT_DISPLAYOBJECT,
		FIT_MOVIECLIP,
	};
	
	// ======================================================
	// = The base item in each frame list, the heart of Swf =
	// ======================================================
	class FrameItem : public AsObject {
	public:
		typedef std::unordered_map<std::string, int> AsPropertyStringMap;
		
		virtual ~FrameItem (){};
		virtual void display( Player* _player, Scene::RenderContext* _ctx ) = 0;
		virtual void advance( float _elapsedMs ) {};
		virtual MovieClip* getAsMovieClip() { return NULL; }
		
		// numeric property handling
		virtual AsObjectHandle getProperty( int _index );
		virtual void setProperty( int _index, AsObjectHandle _val );
		
		virtual float getXScale();
		virtual void setXScale( float _xscale );
		virtual float getYScale();
		virtual void setYScale( float _yscale );
		virtual float getRotation();
		virtual void setRotation( float angle );
			
		virtual AsObjectHandle getProperty( const std::string& _name );
		virtual void setProperty( const std::string& _name, AsObjectHandle _handle );
		
	protected:
		FrameItem ( uint16_t _depth, FrameItemType _type, uint16_t _id, MovieClip* _parent, const std::string& _name ) :
			AsObject( APT_OBJECT ) {
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
		MovieClip* parent;
		bool visible;
		static AsPropertyStringMap* s_asPropertyStringMap;
	};
} /* Swf */


#endif /* _SWFFRAMEITEM_H_ */


