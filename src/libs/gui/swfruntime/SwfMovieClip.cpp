// 
//  SwfMovieClip.cpp	
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/parser.h"
#include "gui/SwfParser/SwfColourTransform.h"
#include "gui/SwfParser/SwfSprite.h"
#include "gui/SwfParser/SwfDisplayObject.h"
#include "gui/SwfParser/SwfFrame.h"
#include "gui/SwfParser/SwfParser.h"
#include "gui/SwfParser/SwfDictionary.h"
#include "SwfRuntimeUtils.h"
#include "SwfPlayer.h"
#include "SwfDisplayObjectFrameItem.h"
#if defined( USE_ACTIONSCRIPT )
#	include "ActionScript/CodeGen/AsVM.h"
#endif
#include "SwfMovieClip.h"

namespace Swf {
	struct Sorter {
		bool operator()( const SwfFrameItem* _a, const SwfFrameItem* _b){
			return (_a->depth < _b->depth);
		}
	};
	
	// ========================================
	// = Private ctor for the root movie clip =
	// ========================================
	SwfMovieClip::SwfMovieClip(SwfPlayer* _player, std::vector<SwfFrame*>* _frames) :
		SwfDisplayObjectFrameItem(NULL, NULL, FIT_MOVIECLIP ) {
			Create(_player, _frames);
	}
	
	SwfMovieClip::SwfMovieClip(SwfDisplayObject* _dobj, SwfPlayer* _player, std::vector<SwfFrame*>* _frames, SwfMovieClip* _parent, SwfSprite* _sprite) : 
		SwfDisplayObjectFrameItem(_dobj, _parent, FIT_MOVIECLIP ) {
			stopped = false;
			Create(_player, _frames);
			if(_sprite->frameCount > 0 ) {
				ProcessFrame( (*_frames)[0] );
			}
	}
	
	// ==========================================
	// = Internal create used by both real ctor =
	// ==========================================
	void SwfMovieClip::Create(SwfPlayer* _player, std::vector<SwfFrame*>*_frames){
		player = _player;
		frames = _frames;
		stopped = false;
		frameABC = NULL;
		currentFrameNumber = 1; // frame numbers are 1 indexed under flash
		accumTimeInMs = 0.0f;
		colourDirty = true;
		transformDirty = true;
	}
	
	void SwfMovieClip::ProcessFrame( const SwfFrame* _frame ) {
/*		if( _frame == (*frames)[0] ) {
			depthToFrameItem.clear();
			sortedArray.clear();
			properties.clear();
		} */
		
		for(	std::list<SwfControlElement*>::const_iterator i = _frame->frameElements.begin();
				i != _frame->frameElements.end();
				++i) {
			switch( (*i)->type ){
			case CE_DISPLAYOBJECT:
				ProcessDisplayObject((SwfDisplayObject*)(*i));
				break;
			case CE_REMOVEOBJECT:
				ProcessRemoveObject((SwfRemoveObject*)(*i));
				break;
			case CE_ACTIONBYTECODE:
				ProcessActionByteCode( _frame, (SwfActionByteCode*)(*i) );
				break;
			default:
				break;
			}
		}
	}
	
	void SwfMovieClip::SortArray(){
		// provide the sorted array
		sortedArray.clear();
		sortedArray.reserve( depthToFrameItem.size() );
		for( 	DepthMap::const_iterator i = depthToFrameItem.begin();
				i != depthToFrameItem.end();
				++i ) {
			sortedArray.push_back( (*i).second );
		}
		
		std::sort( sortedArray.begin(), sortedArray.end(), Sorter() );
	}
	
	void SwfMovieClip::UpdateDisplayObject( SwfDisplayObject* _dobj, DepthMap::const_iterator dIt ){
		// item already exists, so just update it
		SwfFrameItem* item = dIt->second;
		
		if( _dobj->hasMatrix ) {
			if(parent != NULL) {
				item->concatMatrix = Math::MultiplyMatrix( Convert(_dobj->matrix), concatMatrix  );
			} else {
				item->concatMatrix = Convert(_dobj->matrix);
			}
		}
		if (_dobj->hasColourTransform) {
			if (parent != NULL) {
				item->colourTransform = SwfColourTransform::Multiply(_dobj->cxform, colourTransform);
			} else {
				item->colourTransform = _dobj->cxform;
			}
		}
		if( item->type == FIT_MOVIECLIP ) {
			SwfMovieClip* mc = (SwfMovieClip*) item;
			mc->colourDirty = _dobj->hasColourTransform;
			mc->transformDirty = _dobj->hasMatrix;
			mc->UpdateFrame(_dobj->hasMatrix, mc->concatMatrix, _dobj->hasColourTransform, mc->colourTransform);
		}
	}

	void SwfMovieClip::ReplaceDisplayObject(SwfDisplayObject* _dobj, DepthMap::iterator _dIt, uint16_t _did ) {
			
		// item already exists, so update it including the actual object itself
		SwfFrameItem* item = _dIt->second;
		std::string name = item->name;
		Math::Matrix4x4 mat = item->concatMatrix;
		Swf::SwfColourTransform* colourTransform = item->colourTransform;
		depthToFrameItem.erase( _dIt );
		SetProperty( name, AutoGen::AsObjectUndefined::Get() );
		
		// create the new one
		AddDisplayObject(_dobj, _did);
		SwfFrameItem* newItem = depthToFrameItem[_dobj->depth];
		newItem->name = name;
		newItem->concatMatrix = mat;
		newItem->colourTransform = colourTransform;
//		CORE_DELETE item;
		
		UpdateDisplayObject(_dobj, depthToFrameItem.find(_dobj->depth));
	}
	
	void SwfMovieClip::AddDisplayObject(SwfDisplayObject* _dobj, uint16_t _did ) {
		SwfDictionary::CharacterMap::const_iterator chIt = player->parser->dictionary.characters.find(_did);
		if (chIt == player->parser->dictionary.characters.end() )
			return;
	
		SwfFrameItem* item = NULL;
		if( (*chIt).second->type == CT_SPRITE) {
			SwfSprite* sprite = (SwfSprite*) (*chIt).second;
			item = CORE_NEW SwfMovieClip(_dobj, player, &sprite->frames, this, sprite);
		} else {
			item = CORE_NEW SwfDisplayObjectFrameItem(_dobj, this);
		}
		
		depthToFrameItem[_dobj->depth] = item;
		if( !item->name.empty() ) {
			SetProperty( ToLowerIfReq(item->name,player->parser->fileVersion >= 7), item );
		}
	}
	
	void SwfMovieClip::ProcessDisplayObject( SwfDisplayObject* _dobj ){
		uint16_t did = _dobj->id;
		DepthMap::iterator dIt = depthToFrameItem.find( _dobj->depth );

		// if did == 0 and dIt is valid its a move/update record
		if( did == 0 ) {
			if(dIt == depthToFrameItem.end())
				return;
			did = dIt->second->id;
		}
		
		if(dIt != depthToFrameItem.end()) {
			SwfFrameItem* item = dIt->second;
			if( item->id == did) {
				UpdateDisplayObject(_dobj, dIt);
			} else {
				ReplaceDisplayObject(_dobj, dIt, did);
			}
		} else {
			AddDisplayObject(_dobj,did);
		}
	}
	
	void SwfMovieClip::ProcessRemoveObject( SwfRemoveObject* _dobj ){
		DepthMap::iterator dIt = depthToFrameItem.find( _dobj->depth );
		if(dIt == depthToFrameItem.end())
			return;
		SwfFrameItem* oldItem = (*dIt).second;		
		
		depthToFrameItem.erase( dIt );
		SetProperty( ToLowerIfReq(oldItem->name,player->parser->fileVersion >= 7), AutoGen::AsObjectUndefined::Get() );
//		CORE_DELETE oldItem;
	}

	void SwfMovieClip::ProcessActionByteCode( const SwfFrame* _frame, SwfActionByteCode* _abc ) {
		// byte code might refer to items added this frame but not yet
		// so we defer the byte code execution until after all add/remove/update this
		// frame
		frameABC = _abc;
	}
	
	void SwfMovieClip::UpdateFrame( bool _hasMatrix, Math::Matrix4x4 _concat, bool _hasColourCx, SwfColourTransform* _concatCx)
	{
		if(frameABC) {
#if defined( USE_ACTIONSCRIPT )
			player->virtualMachine->ProcessByteCode( this, frameABC );	
#endif
			frameABC = NULL;
		}
				
		for( 	DepthMap::const_iterator i = depthToFrameItem.begin();
				i != depthToFrameItem.end();
				++i ) {
			SwfFrameItem* item = i->second;

			if( item->type == FIT_DISPLAYOBJECT ) {
				SwfDisplayObjectFrameItem* doi = (SwfDisplayObjectFrameItem*) item;
				if ( _hasMatrix ) {
					doi->concatMatrix = Math::MultiplyMatrix(Convert(doi->displayObject->matrix),_concat);
				}
				if ( _hasColourCx) {
					doi->colourTransform = SwfColourTransform::Multiply( doi->displayObject->cxform, _concatCx);
				}
			} else if (item->type == FIT_MOVIECLIP) {
				SwfMovieClip* litem = (SwfMovieClip*) item;
				if ( _hasMatrix ) {
					litem->concatMatrix = Math::MultiplyMatrix(Convert(litem->displayObject->matrix), _concat );
				}
				if ( _hasColourCx) {
					litem->colourTransform = SwfColourTransform::Multiply( litem->displayObject->cxform, _concatCx);
				} 
				litem->UpdateFrame(litem->displayObject->hasMatrix, litem->concatMatrix, litem->displayObject->hasColourTransform, litem->colourTransform);
			}
		}
		SortArray();
	}
    
	void SwfMovieClip::Play()
	{
		stopped = false;
	}

	void SwfMovieClip::Stop()
	{
		stopped = true;
	}

	void SwfMovieClip::NextFrame() {
		// increment
		currentFrameNumber++;
		
		// loop
		if (currentFrameNumber > (int)frames->size()) {
			currentFrameNumber = 1;
		}
	}
	void SwfMovieClip::PrevFrame() {
		// decrement
		currentFrameNumber--;
		
		// loop
		if (currentFrameNumber < 1) {
			currentFrameNumber = 1;
		}
	}
	
	void SwfMovieClip::Process() {
		do {
			if(frameABC) {
#if defined( USE_ACTIONSCRIPT )
				player->virtualMachine->ProcessByteCode( this, frameABC );	
#endif
				frameABC = NULL;
			}
			SortArray();
			for (size_t i = 0; i < sortedArray.size(); i++) {
				SwfFrameItem* item = sortedArray[i];
		    
				if( item->type == FIT_MOVIECLIP ) {
					SwfMovieClip* mc = (SwfMovieClip*)item;
					mc->Process();
				}
			}
			ProcessFrame( (*frames)[currentFrameNumber-1] );
		} while(currentFrameNumber <= (int)frames->size());  
	}

	void SwfMovieClip::GotoFrame(uint16_t _frame) {		
		_frame = std::min(_frame, (uint16_t)(frames->size()));
		currentFrameNumber = _frame;
	}

	void SwfMovieClip::Display( SwfPlayer* _player) {
		// render in depth order
		for (size_t i = 0; i < sortedArray.size(); i++)	{
			SwfFrameItem* item = sortedArray[i];
			if( item->visible ) {
				item->Display(_player);
			}
		}
	}

    void SwfMovieClip::Advance(float _elapsedInMs) {
		if (stopped == false) {
			accumTimeInMs += _elapsedInMs;
			while (accumTimeInMs > player->frameRateInMs) {
				NextFrame();
				ProcessFrame( (*frames)[currentFrameNumber-1] );
				accumTimeInMs -= player->frameRateInMs;
			}
		}
		
		UpdateFrame(transformDirty, concatMatrix, colourDirty, colourTransform);
		transformDirty = false;
		colourDirty = false;

		for (size_t i = 0; i < sortedArray.size(); i++) {
			SwfFrameItem* item = sortedArray[i];
			item->Advance(_elapsedInMs);
		}        
    }

	
	SwfFrameItem* SwfMovieClip::FindTarget(const std::vector<std::string>& _target, int _depth) {
		// can happen due to special paths like ".."
		if (_depth >= (int)_target.size() )
			return this;

		// check special cases
		if(_target[_depth] ==  ".." ) {
			return parent->FindTarget(_target, _depth + 1);
		}
		if(_target[_depth] ==  "_root" ) {
			assert(false);
			return 0;//player.DisplayList.FindTarget(_target, _depth + 1);
		}
		if(_target[_depth] ==  "this") {
			return FindTarget(_target, _depth + 1);
		}
		
		for (size_t i = 0; i < sortedArray.size(); i++) {
			SwfFrameItem* item = sortedArray[i];
			if (item->name == _target[_depth] ) {
				if (_depth >= (int)_target.size()) {
					return item;
				} else {
					return item->GetAsMovieClip()->FindTarget(_target, _depth + 1);
				}
			}
		}
		return NULL;
	}
    
} /* Swf */
