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
		// play and stop control whether advance actually happens, a stopped item can be manually controlled
		virtual void play(){};
		virtual void stop(){};

		virtual MovieClip* getAsMovieClip() { return nullptr; }
		
		// numeric property handling
		virtual AsObjectHandle getProperty( int _index ) const;
		virtual void setProperty( int _index, AsObjectHandle _val );
		
		virtual float getXPosition() const;
		virtual void setXPosition( const float _xpos );
		virtual float getYPosition() const;
		virtual void setYPosition( const float _ypos );
		virtual float getXScale() const;
		virtual void setXScale( const float _xscale );
		virtual float getYScale() const;
		virtual void setYScale( const float _yscale );
		virtual float getRotation() const;
		virtual void setRotation( const float angle );
			
		virtual AsObjectHandle getProperty( const std::string& _name );
		virtual void setProperty( const std::string& _name, AsObjectHandle _handle );
		
	protected:
		FrameItem ( uint16_t _depth, FrameItemType _type, uint16_t _id, MovieClip* _parent, const std::string& _name ) :
			AsObject( APT_OBJECT ),
			depth( _depth ),
			type( _type ),
			id( _id ),
			parent( _parent ),
			name( _name ),
			visible( true ) {}		
	public:		
		const FrameItemType type;
		const uint16_t id;

		std::string name;
		uint16_t depth;
		Math::Matrix4x4 concatMatrix;
		SwfColourTransform* colourTransform;
		MovieClip* parent;
		bool visible;
		static AsPropertyStringMap* s_asPropertyStringMap;
	};
} /* Swf */


#endif /* _SWFFRAMEITEM_H_ */


