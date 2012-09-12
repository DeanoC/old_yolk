// 
//  SwfDisplayObjectFrameItem.h
//  Projects
//  
//  Created by Deano on 2008-09-29.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFDISPLAYOBJECTFRAMEITEM_H_
#define _SWFDISPLAYOBJECTFRAMEITEM_H_

#include "SwfFrameItem.h"
namespace Swf {
	//forward decl
	class SwfDisplayObject;
	class SwfMovieClip;
	class SwfPlayer;
	
	class SwfDisplayObjectFrameItem : public SwfFrameItem {
	public:
		SwfDisplayObjectFrameItem (SwfDisplayObject* _object, SwfMovieClip* _parent);

		virtual float GetXScale() override;
		virtual float GetYScale() override;
		virtual float GetRotation() override;
		virtual void SetRotation(float angle) override;

		virtual void Display(SwfPlayer* _player) override;
		SwfDisplayObject* displayObject;
	protected:
		SwfDisplayObjectFrameItem (SwfDisplayObject* _object, SwfMovieClip* _parent, FrameItemType _type);
		void Create();
	};
} /* Swf */


#endif /* _SWFDISPLAYOBJECTFRAMEITEM_H_ */


