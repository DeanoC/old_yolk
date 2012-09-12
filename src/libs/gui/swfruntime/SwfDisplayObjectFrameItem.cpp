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
#include "SwfRuntimeUtils.h"
#include "SwfMovieClip.h"
#include "SwfRuntimeCharacter.h"
#include "SwfPlayer.h"
#include "SwfDisplayObjectFrameItem.h"

namespace Swf {
	float SwfDisplayObjectFrameItem::GetXScale() {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return sqrt(lm._11 * lm._11 + lm._12 * lm._12);
	}
	
	float SwfDisplayObjectFrameItem::GetYScale() {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return sqrt(lm._22 * lm._22 + lm._21 * lm._21);
	}

	float SwfDisplayObjectFrameItem::GetRotation() {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return Math::radian_to_degree<float>() * atan2(lm._21, lm._11);
	}

	void SwfDisplayObjectFrameItem::SetRotation(float angle) {
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
	
	SwfDisplayObjectFrameItem::SwfDisplayObjectFrameItem (SwfDisplayObject* _object, SwfMovieClip* _parent) :
		SwfFrameItem(_object->depth, FIT_DISPLAYOBJECT, _object->id, _parent, _object->name),
		displayObject( _object ) {
		Create();
	}
	
	SwfDisplayObjectFrameItem::SwfDisplayObjectFrameItem (SwfDisplayObject* _object, SwfMovieClip* _parent, FrameItemType _type) :
		SwfFrameItem( _object ? _object->depth : 0, _type, _object ? _object->id : 0, _parent, _object ? _object->name : "_root" ),
		displayObject( _object ) {
		Create();
	}
	
	void SwfDisplayObjectFrameItem::Create() {
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
	
	void SwfDisplayObjectFrameItem::Display(SwfPlayer* _player) {
		SwfRuntimeCharacter* chara = _player->getCharacter(id);
		if(chara == NULL) {
			return;
		}

		if (displayObject->hasClipDepth) {
			chara->Display(_player, this, &concatMatrix, colourTransform, displayObject->clipDepth, true, displayObject->morphRatio);
		} else {
			chara->Display(_player, this, &concatMatrix, colourTransform, depth, false, displayObject->morphRatio);
		}
	}
} /* Swf */
