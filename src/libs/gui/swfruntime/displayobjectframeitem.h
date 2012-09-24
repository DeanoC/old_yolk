// 
//  SwfDisplayObjectFrameItem.h
//  Projects
//  
//  Created by Deano on 2008-09-29.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_DISPLAYOBJECTFRAMEITEM_H_
#define YOLK_GUI_SWFRUNTIME_DISPLAYOBJECTFRAMEITEM_H_

#include "frameitem.h"

namespace Swf {
	//forward decl
	class SwfDisplayObject;
	class MovieClip;
	class Player;
	
	class DisplayObjectFrameItem : public FrameItem {
	public:
		DisplayObjectFrameItem ( SwfDisplayObject* _object, MovieClip* _parent );

		virtual float getXPosition() const override;
		virtual void setXPosition( const float _xpos ) override;
		virtual float getYPosition() const override;
		virtual void setYPosition( const float _ypos ) override;
		virtual float getXScale() const override;
		virtual void setXScale( const float _xscale ) override;
		virtual float getYScale() const override;
		virtual void setYScale( const float _yscale ) override;
		virtual float getRotation() const override;
		virtual void setRotation( const float angle ) override;

		virtual void display( Player* _player, Scene::RenderContext* _ctx ) override;
		SwfDisplayObject* displayObject;
	protected:
		void setTransform(float _xscale, float _yscale, float _angle, float _x, float _y );

		DisplayObjectFrameItem ( SwfDisplayObject* _object, MovieClip* _parent, FrameItemType _type);
		void create();
	};
} /* Swf */


#endif /* YOLK_GUI_SWFRUNTIME_DISPLAYOBJECTFRAMEITEM_H_ */


