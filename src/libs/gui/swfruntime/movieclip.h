// 
//  SwfMovieClip.h
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFMOVIECLIP_H_
#define _SWFMOVIECLIP_H_

#include "frameitem.h"
#include "displayobjectframeitem.h"

#include "actionscript/autogen/AsObject.h"

namespace Swf {
	// forward decl
	class SwfFrame;
	class SwfSprite;
	class SwfDisplayObject;
	class SwfRemoveObject;
	class SwfActionByteCode;

	class Player;
	
	// ===================================================
	// = MovieClip encapsulates a Sprite/DisplayList  =
	// ===================================================
	class MovieClip : public DisplayObjectFrameItem {
	public:
		virtual ~MovieClip (){};

		static MovieClip* createRoot( Player* _player, std::vector<SwfFrame*>* _frames ) {
			return CORE_NEW MovieClip( _player, _frames );
		}

		MovieClip( SwfDisplayObject* _dobj, Player* _player, std::vector<SwfFrame*>* _frames, MovieClip* _parent, SwfSprite* _sprite);
		
		void processFrame( const SwfFrame* _frame );

		void play();
		void stop();
		void nextFrame();
		void prevFrame();
		void gotoFrame(uint16_t _frame);
		void process();
				
		int getCurrentFrameNumber() const {
			return currentFrameNumber;
		}
		int getMaximumFrameNumber() const {
			return frames->size();
		}
		
		Player* getPlayer() {
			return player;
		}
		virtual void display( Player* _player, Scene::RenderContext* _ctx ) override;
		virtual void advance( float _elapsedInMs ) override;

		virtual void setRotation( float angle ) override {
			DisplayObjectFrameItem::setRotation(angle);
			transformDirty = true;
		}
		
		virtual MovieClip* getAsMovieClip() { return this; }
		FrameItem* findTarget( const std::vector<std::string>& _target, int _depth );

	private:
		typedef Core::gcmap<uint16_t, FrameItem*> DepthMap;
		typedef Core::gcvector<FrameItem*> DepthSortedArray;
		
		void processDisplayObject( SwfDisplayObject* _dobj );
		void processRemoveObject( SwfRemoveObject* _dobj );
		void processActionByteCode( const SwfFrame* _frame, SwfActionByteCode* _abc );
		
		void addDisplayObject( SwfDisplayObject* _dobj, uint16_t _did );
		void updateDisplayObject( SwfDisplayObject* _dobj, DepthMap::const_iterator dIt );
		void replaceDisplayObject( SwfDisplayObject* _dobj, uint16_t _did );
		void replaceDisplayObject( SwfDisplayObject* _dobj, DepthMap::iterator _dIt, uint16_t _did );
		void processABC();

		MovieClip ( Player* _player, std::vector<SwfFrame*>* _frameList );
		
		void create( Player* _player, std::vector<SwfFrame*>*_frames );
		void updateFrame( bool _hasMatrix, Math::Matrix4x4 _concat, bool _hasColourCx, SwfColourTransform* _concatCx );
		void sortArray();

		// data
		Player* player;
		std::vector<SwfFrame*>* frames;

		DepthMap 			depthToFrameItem;
		DepthSortedArray 	sortedArray;
		
		bool 				stopped;
		int 				currentFrameNumber;
		double 				accumTimeInMs;
		bool 				colourDirty;
		bool 				transformDirty;
		SwfActionByteCode*	frameABC;
	};
} /* Swf */


#endif /* _SWFMOVIECLIP_H_ */

