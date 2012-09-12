// 
//  SwfMovieClip.h
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFMOVIECLIP_H_
#define _SWFMOVIECLIP_H_

#include "SwfFrameItem.h"
#include "SwfDisplayObjectFrameItem.h"

#include "actionscript/autogen/AsObject.h"

namespace Swf {
	// forward decl
	class SwfFrame;
	class SwfPlayer;
	class SwfSprite;
	class SwfDisplayObject;
	class SwfRemoveObject;
	class SwfActionByteCode;
	
	// ===================================================
	// = SwfMovieClip encapsulates a Sprite/DisplayList  =
	// ===================================================
	class SwfMovieClip : public SwfDisplayObjectFrameItem {
	public:
		virtual ~SwfMovieClip (){};

		static SwfMovieClip* CreateRoot(SwfPlayer* _player, std::vector<SwfFrame*>* _frames) {
			return CORE_NEW SwfMovieClip(_player, _frames);
		}

		SwfMovieClip(SwfDisplayObject* _dobj, SwfPlayer* _player, std::vector<SwfFrame*>* _frames, SwfMovieClip* _parent, SwfSprite* _sprite);
		
		void ProcessFrame( const SwfFrame* _frame );

		void Play();
		void Stop();
		void NextFrame();
		void PrevFrame();
		void GotoFrame(uint16_t _frame);
		void Process();
				
		int GetCurrentFrameNumber() const {
			return currentFrameNumber;
		}
		int GetMaximumFrameNumber() const {
			return frames->size();
		}
		
		SwfPlayer* GetPlayer() {
			return player;
		}
		virtual void Display(SwfPlayer* _player);
		virtual void Advance(float _elapsedInMs);

		virtual void SetRotation( float angle ) override {
			SwfDisplayObjectFrameItem::SetRotation(angle);
			transformDirty = true;
		}
		
		virtual SwfMovieClip* GetAsMovieClip() { return this; }
		SwfFrameItem* FindTarget(const std::vector<std::string>& _target, int _depth);

	private:
		typedef Core::gcmap<uint16_t, SwfFrameItem*> DepthMap;
		typedef Core::gcvector<SwfFrameItem*> DepthSortedArray;
		
		void ProcessDisplayObject( SwfDisplayObject* _dobj );
		void ProcessRemoveObject( SwfRemoveObject* _dobj );
		void ProcessActionByteCode( const SwfFrame* _frame, SwfActionByteCode* _abc );
		
		void AddDisplayObject(SwfDisplayObject* _dobj, uint16_t _did );
		void UpdateDisplayObject( SwfDisplayObject* _dobj, DepthMap::const_iterator dIt );
		void ReplaceDisplayObject(SwfDisplayObject* _dobj, uint16_t _did );
		void ReplaceDisplayObject(SwfDisplayObject* _dobj, DepthMap::iterator _dIt, uint16_t _did );
		void ProcessABC();

		SwfMovieClip (SwfPlayer* _player, std::vector<SwfFrame*>* _frameList);
		
		void Create(SwfPlayer* _player, std::vector<SwfFrame*>*_frames);
		void UpdateFrame( bool _hasMatrix, Math::Matrix4x4 _concat, bool _hasColourCx, SwfColourTransform* _concatCx);
		void SortArray();

		// data
		SwfPlayer* player;
		std::vector<SwfFrame*>* frames;

		DepthMap 			depthToFrameItem;
		DepthSortedArray 	sortedArray;
		
		bool 				stopped;
		int 				currentFrameNumber;
		double 			accumTimeInMs;
		bool 				colourDirty;
		bool 				transformDirty;
		SwfActionByteCode*	frameABC;
	};
} /* Swf */


#endif /* _SWFMOVIECLIP_H_ */

