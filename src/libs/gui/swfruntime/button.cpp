/*
 *  button.cpp
 *
 *  Copyright 2013 Cloud Pixies Ltd. All rights reserved.
 *
 */

#include "swfruntime.h"
#include "gui/swfparser/SwfDisplayObject.h"
#include "gui/swfparser/SwfButton.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "gui/swfparser/parser.h"
#include "gui/swfparser/SwfCharacter.h"
#include "gui/swfparser/SwfShape.h"
#include "player.h"
#include "shape.h"
#include "utils.h"
#include "button.h"

namespace Swf {

	Button::Button( SwfDisplayObject* _dobj, SwfButton* _button, MovieClip* _parent ) : 
		DisplayObjectFrameItem( _dobj, _parent, FIT_BUTTON ),
		hitRecord( nullptr ),
		button( _button ),
		currentState( IDLE )
	{
		// find the hit detect record (presume only one?)
		for( int i = 0; i < button->numRecords; ++i ) {
			if( button->records[i].buttonStateHitTest ) {
				hitRecord = &button->records[i];
			}
		}
		for( int i = 0; i < MAX_STATES; ++i ) {
			for( int j = 0; j < MAX_STATES; ++ j ) {
				transitionTable[ i ] [ j ] = nullptr;
			}
		}

#define TT( N, O, C ) if( button->condActions[i]. N ) { \
						CORE_ASSERT( transitionTable[ (O) ][ (C) ] == nullptr ); \
						transitionTable[ (O) ][ (C) ] = &button->condActions[i]; \
		}

		for( int i = 0; i < button->numCondActions; ++i ) {
			TT( idleToOverDown, IDLE , OVER | DOWN );
			TT( overDownToIdle, OVER | DOWN , IDLE );
			TT( outDownToOverDown, DOWN , OVER | DOWN );
			TT( overDownToOutDown, OVER | DOWN , DOWN  );
			TT( overDownToOverUp, OVER | DOWN , OVER  );
			TT( overUpToOverDown, OVER , OVER | DOWN );
			TT( outDownToIdle, DOWN , IDLE  );
			TT( overUpToIdle, OVER , IDLE  );
			TT( idleToOverUp, IDLE , OVER  );
		}
#undef TT
	}
	Button::~Button() {
		CORE_DELETE( button );
	}

	void Button::display( const Player* _player, Scene::RenderContext* _ctx ) {
		DisplayObjectFrameItem::display( _player, _ctx );

		for( int i = 0; i < button->numRecords; ++i ) {
			// ignore hitRecord
			if( &button->records[i] == hitRecord ) continue;

			// down inside
			if( button->records[i].buttonStateDown && (currentState == (OVER|DOWN)) ) {
				displayRecord( i, _player, _ctx );
			} 
			// over inside
			if( button->records[i].buttonStateOver && (currentState == OVER) ) {
				displayRecord( i, _player, _ctx );
			}
			// up or down outside hit rect
			if( button->records[i].buttonStateUp && ((currentState == IDLE) || (currentState == DOWN)) ) {
				displayRecord( i, _player, _ctx );
			}
		}
	}

	void Button::displayRecord( int _recI, const Player* _player, Scene::RenderContext* _ctx ) {

		SwfButtonRecord& rec = button->records[ _recI ];
		Character* chara = _player->getCharacter( rec.characterId );
		if(chara == nullptr) {
			return;
		}

		Math::Matrix4x4 recMat = Math::MultiplyMatrix( Convert(rec.matrix), concatMatrix  );
		SwfColourTransform* cxForm = SwfColourTransform::Multiply( rec.cxform, colourTransform);

		chara->display( _player, _ctx, this, &recMat, cxForm, depth + rec.placeDepth, false, displayObject->morphRatio);

		CORE_DELETE( cxForm );
	}

	bool Button::testHitRecord( const Player* _player, const Math::Vector2& _pt  ) {
		if( hitRecord == nullptr ) return false;

		// TODO check hit record rather than just bounding box
		const SwfCharacter* chara = _player->parser->dictionary.characters[ hitRecord->characterId ];
		if( chara->type == CT_SHAPEOBJECT ) {
			const SwfShapeObject* shape = (SwfShapeObject*) chara;
			Math::Matrix4x4 recMat;
			recMat = Math::MultiplyMatrix( Convert(hitRecord->matrix), concatMatrix  );
			recMat = Math::MultiplyMatrix( recMat, _player->getTwipToNdx() );

			Math::Vector2 verts[4];
			verts[0] = Math::Vector2( (float)shape->bounds->minX, (float)shape->bounds->minY );
			verts[1] = Math::Vector2( (float)shape->bounds->maxX, (float)shape->bounds->minY );
			verts[2] = Math::Vector2( (float)shape->bounds->maxX, (float)shape->bounds->maxY );
			verts[3] = Math::Vector2( (float)shape->bounds->minX, (float)shape->bounds->maxY );
			verts[0] = Math::TransformAndDropZ( verts[0], recMat );
			verts[1] = Math::TransformAndDropZ( verts[1], recMat );
			verts[2] = Math::TransformAndDropZ( verts[2], recMat );
			verts[3] = Math::TransformAndDropZ( verts[3], recMat );
			return Math::ptInPoly( 4, verts, _pt );
		} else {
			return false;
		}
	}

	void Button::updateState( Player* _player, const Math::Vector2& mouseTwip, bool leftButton, bool rightButton ) {
		STATE lastState = currentState;
		if( leftButton ) {
			currentState = DOWN;
		} else {
			currentState = IDLE;
		}

		if( testHitRecord( _player, mouseTwip ) ) {
			currentState = (STATE)( currentState | OVER );
		}

		if( transitionTable[ lastState ][ currentState ] != nullptr ) {
			_player->scheduleActionByteCodeRun( transitionTable[ lastState ][ currentState ]->actionScript );
		}
	}

}