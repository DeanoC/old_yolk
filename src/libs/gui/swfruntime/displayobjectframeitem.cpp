/*
 *  SwfDisplayObjectFrameItem.cpp
 *  SkyGlow-Iphone
 *
 *  Created by Deano on 29/09/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "swfruntime.h"
#include "gui/SwfParser/SwfDisplayObject.h"
#include "gui/SwfParser/SwfColourTransform.h"
#include "gui/SwfParser/SwfMatrix.h"
#include "character.h"
#include "movieclip.h"
#include "player.h"
#include "utils.h"
#include "displayobjectframeitem.h"

namespace Swf {
	float DisplayObjectFrameItem::getXScale() {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return sqrt(lm._11 * lm._11 + lm._12 * lm._12);
	}
	
	float DisplayObjectFrameItem::getYScale() {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return sqrt(lm._22 * lm._22 + lm._21 * lm._21);
	}

	float DisplayObjectFrameItem::getRotation() {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return Math::radian_to_degree<float>() * atan2(lm._21, lm._11);
	}

	void DisplayObjectFrameItem::setRotation(float angle) {
		angle = Math::degree_to_radian<float>() * angle;
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		float xscale = sqrt(lm._11 * lm._11 + lm._12 * lm._12);
		float yscale = sqrt(lm._22 * lm._22 + lm._21 * lm._21);
		float cosAngle = cos(angle);
		float sinAngle = sin(angle);
		lm._11 = (float)(xscale * cosAngle);
		lm._21 = (float)(yscale * -sinAngle);
		lm._12 = (float)(xscale * sinAngle);
		lm._22 = (float)(yscale * cosAngle);
		concatMatrix = Math::MultiplyMatrix( lm, parent->concatMatrix );
		displayObject->matrix->scaleX = lm._11;
		displayObject->matrix->scaleY = lm._22;
		displayObject->matrix->rotateSkew0 = lm._12;
		displayObject->matrix->rotateSkew1 = lm._21;
	}
	
	DisplayObjectFrameItem::DisplayObjectFrameItem (SwfDisplayObject* _object, MovieClip* _parent) :
		FrameItem( _object->depth, FIT_DISPLAYOBJECT, _object->id, _parent, _object->name ),
		displayObject( _object ) {
		create();
	}
	
	DisplayObjectFrameItem::DisplayObjectFrameItem (SwfDisplayObject* _object, MovieClip* _parent, FrameItemType _type) :
		FrameItem( _object ? _object->depth : 0, _type, _object ? _object->id : 0, _parent, _object ? _object->name : "_root" ),
		displayObject( _object ) {
		create();
	}
	
	void DisplayObjectFrameItem::create() {
		Math::Matrix4x4 lm;
		SwfColourTransform* lct;
		if( displayObject ) {
			lm = Convert(displayObject->matrix);
			lct = displayObject->cxform;
		} else {
			lm = Math::IdentityMatrix();
			lct = CORE_NEW SwfColourTransform();
		}
		
		if (parent == NULL) {
			concatMatrix = lm;
			colourTransform = lct;
		} else {
			concatMatrix = Math::MultiplyMatrix( lm, parent->concatMatrix );
			colourTransform = SwfColourTransform::Multiply( lct, parent->colourTransform);
		}
	}
	
	void DisplayObjectFrameItem::display(Player* _player, Scene::RenderContext* _ctx ) {
		Character* chara = _player->getCharacter(id);
		if(chara == NULL) {
			return;
		}

		if (displayObject->hasClipDepth) {
			chara->display( _player, _ctx, this, &concatMatrix, colourTransform, displayObject->clipDepth, true, displayObject->morphRatio);
		} else {
			chara->display( _player, _ctx, this, &concatMatrix, colourTransform, depth, false, displayObject->morphRatio);
		}
	}
} /* Swf */
