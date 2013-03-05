// 
//  SwfMovieClip.cpp	
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/parser.h"
#include "gui/SwfParser/SwfButton.h"
#include "gui/SwfParser/SwfColourTransform.h"
#include "gui/SwfParser/SwfSprite.h"
#include "gui/SwfParser/SwfDisplayObject.h"
#include "gui/SwfParser/SwfFrame.h"
#include "gui/SwfParser/SwfParser.h"
#include "gui/SwfParser/SwfDictionary.h"
#include "ActionScript/AsVM.h"
#include "displayobjectframeitem.h"
#include "button.h"
#include "player.h"
#include "utils.h"
#include "movieclip.h"

namespace Swf {
	struct Sorter {
		bool operator()( const FrameItem* _a, const FrameItem* _b){
			return (_a->depth < _b->depth);
		}
	};
	
	// ========================================
	// = Private ctor for the root movie clip =
	// ========================================
	MovieClip::MovieClip( Player* _player, std::vector<SwfFrame*>* _frames ) :
		DisplayObjectFrameItem(NULL, NULL, FIT_MOVIECLIP ) {
			create(_player, _frames);
	}
	
	MovieClip::MovieClip( SwfDisplayObject* _dobj, Player* _player, std::vector<SwfFrame*>* _frames, MovieClip* _parent, SwfSprite* _sprite) : 
		DisplayObjectFrameItem(_dobj, _parent, FIT_MOVIECLIP ) {
			stopped = false;
			create(_player, _frames);
			if(_sprite->frameCount > 0 ) {
				processFrame( (*_frames)[0] );
			}
	}
	
	// ==========================================
	// = Internal create used by both real ctor =
	// ==========================================
	void MovieClip::create( Player* _player, std::vector<SwfFrame*>*_frames ) {
		player = _player;
		frames = _frames;
		stopped = false;
		currentFrameNumber = 1; // frame numbers are 1 indexed under flash
		accumTimeInMs = 0.0f;
		colourDirty = true;
		transformDirty = true;
	}

	MovieClip::~MovieClip () {
		if( frames != nullptr ) {
			for( auto i : *frames ) {
				CORE_DELETE( i );
			}
		}

		for( auto i : depthToFrameItem ) {
			CORE_DELETE( i.second );
		}
	}

	
	void MovieClip::processFrame( const SwfFrame* _frame ) {
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
				processDisplayObject((SwfDisplayObject*)(*i));
				break;
			case CE_REMOVEOBJECT:
				processRemoveObject((SwfRemoveObject*)(*i));
				break;
			case CE_ACTIONBYTECODE:
				processActionByteCode( _frame, (SwfActionByteCode*)(*i) );
				break;
			default:
				break;
			}
		}
	}
	
	void MovieClip::sortArray(){
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
	
	void MovieClip::updateDisplayObject( SwfDisplayObject* _dobj, DepthMap::const_iterator dIt ){
		// item already exists, so just update it
		FrameItem* item = dIt->second;
		
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
			MovieClip* mc = (MovieClip*) item;
			mc->colourDirty = _dobj->hasColourTransform;
			mc->transformDirty = _dobj->hasMatrix;
			mc->updateFrame(_dobj->hasMatrix, mc->concatMatrix, _dobj->hasColourTransform, mc->colourTransform);
		}
	}

	void MovieClip::replaceDisplayObject(SwfDisplayObject* _dobj, DepthMap::iterator _dIt, uint16_t _did ) {
			
		// item already exists, so update it including the actual object itself
		FrameItem* item = _dIt->second;
		std::string name = item->name;
		Math::Matrix4x4 mat = item->concatMatrix;
		SwfColourTransform* colourTransform = item->colourTransform;
		depthToFrameItem.erase( _dIt );
		setProperty( name, AsObjectUndefined::get() );
		
		// create the new one
		addDisplayObject(_dobj, _did);
		FrameItem* newItem = depthToFrameItem[_dobj->depth];
		newItem->name = name;
		newItem->concatMatrix = mat;
		newItem->colourTransform = colourTransform;
		CORE_DELETE item;
		
		updateDisplayObject(_dobj, depthToFrameItem.find(_dobj->depth));
	}
	
	void MovieClip::addDisplayObject(SwfDisplayObject* _dobj, uint16_t _did ) {
		SwfDictionary::CharacterMap::const_iterator chIt = player->parser->dictionary.characters.find(_did);
		if (chIt == player->parser->dictionary.characters.end() )
			return;
	
		FrameItem* item = NULL;
		switch( (*chIt).second->type ) {
		case CT_SPRITE: {
			SwfSprite* sprite = (SwfSprite*) (*chIt).second;
			item = CORE_NEW MovieClip(_dobj, player, &sprite->frames, this, sprite);
			break;
			}
		case CT_BUTTON: {
			SwfButton* button = (SwfButton*) (*chIt).second;
			item = CORE_NEW Button(_dobj, button, this);
			break;
			}
		default:
			item = CORE_NEW DisplayObjectFrameItem(_dobj, this);
			break;
		}

		depthToFrameItem[_dobj->depth] = item;
		if( !item->name.empty() ) {
			setProperty( ToLowerIfReq(item->name,player->parser->fileVersion >= 7), item );
		}
	}
	
	void MovieClip::processDisplayObject( SwfDisplayObject* _dobj ){
		uint16_t did = _dobj->id;
		DepthMap::iterator dIt = depthToFrameItem.find( _dobj->depth );

		// if did == 0 and dIt is valid its a move/update record
		if( did == 0 ) {
			if(dIt == depthToFrameItem.end())
				return;
			did = dIt->second->id;
		}
		
		if(dIt != depthToFrameItem.end()) {
			FrameItem* item = dIt->second;
			if( item->id == did) {
				updateDisplayObject(_dobj, dIt);
			} else {
				replaceDisplayObject(_dobj, dIt, did);
			}
		} else {
			addDisplayObject(_dobj,did);
		}
	}
	
	void MovieClip::processRemoveObject( SwfRemoveObject* _dobj ){
		DepthMap::iterator dIt = depthToFrameItem.find( _dobj->depth );
		if(dIt == depthToFrameItem.end())
			return;
		FrameItem* oldItem = (*dIt).second;		
		
		depthToFrameItem.erase( dIt );
		setProperty( ToLowerIfReq(oldItem->name,player->parser->fileVersion >= 7), AsObjectUndefined::get() );
		CORE_DELETE oldItem;
	}

	void MovieClip::processActionByteCode( const SwfFrame* _frame, SwfActionByteCode* _abc ) {
		// byte code might refer to items added this frame but not yet
		// so we defer the byte code execution until after all add/remove/update this
		// frame
		player->scheduleActionByteCodeRun( _abc );
	}
	
	void MovieClip::updateFrame( bool _hasMatrix, Math::Matrix4x4 _concat, bool _hasColourCx, SwfColourTransform* _concatCx) {
				
		for( 	DepthMap::const_iterator i = depthToFrameItem.begin();
				i != depthToFrameItem.end();
				++i ) {
			FrameItem* item = i->second;
			// TODO why frameitem and displayobjectframeitem..?
			if( item->type == FIT_BUTTON || item->type == FIT_DISPLAYOBJECT || item->type == FIT_MOVIECLIP ) {
				DisplayObjectFrameItem* doi = (DisplayObjectFrameItem*) item;
				if ( _hasMatrix ) {
					doi->concatMatrix = Math::MultiplyMatrix(Convert(doi->displayObject->matrix),_concat);
				}
				if ( _hasColourCx) {
					doi->colourTransform = SwfColourTransform::Multiply( doi->displayObject->cxform, _concatCx);
				}
			}

			if( item->type == FIT_BUTTON ) {
				Button* b = (Button*) item;
				// TODO need a full event state updator...
				b->updateState( player, mouseTwip, false, false ); /*leftButton, rightButton ); */
			} else if (item->type == FIT_MOVIECLIP) {
				MovieClip* litem = (MovieClip*) item;
				litem->updateFrame(litem->displayObject->hasMatrix, litem->concatMatrix, litem->displayObject->hasColourTransform, litem->colourTransform);
			}
		}
		sortArray();
	}
    
	void MovieClip::play() {
		stopped = false;
	}

	void MovieClip::stop() {
		stopped = true;
	}

	void MovieClip::nextFrame() {
		// increment
		currentFrameNumber++;
		
		// loop
		if (currentFrameNumber > (int)frames->size()) {
			currentFrameNumber = 1;
		}
	}
	void MovieClip::prevFrame() {
		// decrement
		currentFrameNumber--;
		
		// loop
		if (currentFrameNumber < 1) {
			currentFrameNumber = 1;
		}
	}
	
	void MovieClip::process() {
		do {
//			if(frameABC) {
//				player->virtualMachine->processByteCode( this, frameABC );	
//				frameABC = NULL;
//			}
			sortArray();
			for (size_t i = 0; i < sortedArray.size(); i++) {
				FrameItem* item = sortedArray[i];
		    
				if( item->type == FIT_MOVIECLIP ) {
					MovieClip* mc = (MovieClip*)item;
					mc->process();
				}
			}
			processFrame( (*frames)[currentFrameNumber-1] );
		} while(currentFrameNumber <= (int)frames->size());  
	}

	void MovieClip::gotoFrame(uint16_t _frame) {		
		_frame = std::min(_frame, (uint16_t)(frames->size()));
		currentFrameNumber = _frame;
	}

	void MovieClip::display( const Player* _player, Scene::RenderContext* _ctx ) {
		// render in depth order
		for (size_t i = 0; i < sortedArray.size(); i++)	{
			FrameItem* item = sortedArray[i];
			if( item->visible ) {
				item->display( _player, _ctx );
			}
		}
	}

    void MovieClip::advance(float _elapsedInMs) {
		if (stopped == false) {
			accumTimeInMs += _elapsedInMs;
			while (accumTimeInMs > player->frameRateInMs) {
				nextFrame();
				processFrame( (*frames)[currentFrameNumber-1] );
				accumTimeInMs -= player->frameRateInMs;
			}
		}
		
		updateFrame(transformDirty, concatMatrix, colourDirty, colourTransform);
		transformDirty = false;
		colourDirty = false;

		for (size_t i = 0; i < sortedArray.size(); i++) {
			FrameItem* item = sortedArray[i];
			item->advance(_elapsedInMs);
		}        
    }

	
	FrameItem* MovieClip::findTarget( const std::vector<std::string>& _target, int _depth ) {
		// can happen due to special paths like ".."
		if (_depth >= (int)_target.size() )
			return this;

		// check special cases
		if( _target[_depth] ==  ".." ) {
			return parent->findTarget(_target, _depth + 1);
		}
		if( _target[_depth] ==  "_root" ) {
			return player->getRoot()->findTarget(_target, _depth + 1);
		}
		if( _target[_depth] ==  "this" ) {
			return findTarget(_target, _depth + 1);
		}
		
		for ( size_t i = 0; i < sortedArray.size(); i++ ) {
			FrameItem* item = sortedArray[i];
			if ( item->name == _target[_depth] ) {
				if (_depth >= (int)_target.size()) {
					return item;
				} else {
					if( item->getAsMovieClip() != nullptr ) {
						return item->getAsMovieClip()->findTarget(_target, _depth + 1);
					} else {
						return nullptr;
					}
				}
			}
		}
		return nullptr;
	}

	FrameItem* MovieClip::findLocalTarget( const std::string& _target ) {
		// TODO replace linear array search with (hash) map
		for ( size_t i = 0; i < sortedArray.size(); i++ ) {
			FrameItem* item = sortedArray[ i ];
			if ( item->name == _target ) {
				return item;
			}
		}
		return nullptr;
	}



	void MovieClip::setMouseInput( float x, float y, bool leftButton, bool rightButton ) {
		mouseTwip = Math::Vector2(x,y);
		// TODO check buttons
	}
    
} /* Swf */
