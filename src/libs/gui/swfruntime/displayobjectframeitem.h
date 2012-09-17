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

		virtual float getXScale() override;
		virtual float getYScale() override;
		virtual float getRotation() override;
		virtual void setRotation( float angle ) override;

		virtual void display( Player* _player, Scene::RenderContext* _ctx ) override;
		SwfDisplayObject* displayObject;
	protected:
		DisplayObjectFrameItem ( SwfDisplayObject* _object, MovieClip* _parent, FrameItemType _type);
		void create();
	};
} /* Swf */


#endif /* YOLK_GUI_SWFRUNTIME_DISPLAYOBJECTFRAMEITEM_H_ */


