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

		float getXPosition() const override;
		void setXPosition( const float _xpos ) override;
		float getYPosition() const override;
		void setYPosition( const float _ypos ) override;
		float getXScale() const override;
		void setXScale( const float _xscale ) override;
		float getYScale() const override;
		void setYScale( const float _yscale ) override;
		float getRotation() const override;
		void setRotation( const float angle ) override;

		void display( const Player* _player, Scene::RenderContext* _ctx ) override;
		SwfDisplayObject* displayObject;
	protected:
		void setTransform(float _xscale, float _yscale, float _angle, float _x, float _y );

		DisplayObjectFrameItem ( SwfDisplayObject* _object, MovieClip* _parent, FrameItemType _type);
		void create();
	};
} /* Swf */


#endif /* YOLK_GUI_SWFRUNTIME_DISPLAYOBJECTFRAMEITEM_H_ */


