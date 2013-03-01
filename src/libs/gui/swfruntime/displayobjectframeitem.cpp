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
	float DisplayObjectFrameItem::getXPosition() const {
		return displayObject->matrix->translateX;
	}

	void DisplayObjectFrameItem::setXPosition( const float _xpos ) {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		float xscale = sqrt(lm._11 * lm._11 + lm._12 * lm._12);
		float yscale = sqrt(lm._22 * lm._22 + lm._21 * lm._21);
		float rangle = atan2(lm._21, lm._11);
		setTransform( xscale, yscale, rangle, _xpos, lm._42 );
	}

	float DisplayObjectFrameItem::getYPosition() const {
		return displayObject->matrix->translateY;
	}

	void DisplayObjectFrameItem::setYPosition( const float _ypos ) {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		float xscale = sqrt(lm._11 * lm._11 + lm._12 * lm._12);
		float yscale = sqrt(lm._22 * lm._22 + lm._21 * lm._21);
		float rangle = atan2(lm._21, lm._11);
		setTransform( xscale, yscale, rangle, lm._41, _ypos );
	}

	float DisplayObjectFrameItem::getXScale() const {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return sqrt(lm._11 * lm._11 + lm._12 * lm._12);
	}
	void DisplayObjectFrameItem::setXScale( float _xscale ) {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		float rangle = atan2(lm._21, lm._11);
		float yscale = sqrt(lm._22 * lm._22 + lm._21 * lm._21);
		setTransform( _xscale, yscale, rangle, lm._41, lm._42 );
	}
	
	float DisplayObjectFrameItem::getYScale() const {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return sqrt(lm._22 * lm._22 + lm._21 * lm._21);
	}

	void DisplayObjectFrameItem::setYScale( float _yscale ) {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		float rangle = atan2(lm._21, lm._11);
		float xscale = sqrt(lm._11 * lm._11 + lm._12 * lm._12);
		setTransform( xscale, _yscale, rangle, lm._41, lm._42 );
	}

	float DisplayObjectFrameItem::getRotation() const {
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		return Math::radian_to_degree<float>() * atan2(lm._21, lm._11);
	}

	void DisplayObjectFrameItem::setRotation(float _angle) {
		float rangle = Math::degree_to_radian<float>() * _angle;
		Math::Matrix4x4 lm = Convert(displayObject->matrix);
		float xscale = sqrt(lm._11 * lm._11 + lm._12 * lm._12);
		float yscale = sqrt(lm._22 * lm._22 + lm._21 * lm._21);
		setTransform( xscale, yscale, rangle, lm._41, lm._42 );
	}

	void DisplayObjectFrameItem::setTransform(float _xscale, float _yscale, float _angle, float _x, float _y ) {
		float cosAngle = cos( _angle );
		float sinAngle = sin( _angle );
		Math::Matrix4x4 lm = Math::IdentityMatrix();
		lm._11 = (float)( _xscale * cosAngle );
		lm._21 = (float)( _yscale * sinAngle );
		lm._12 = (float)( _xscale * -sinAngle );
		lm._22 = (float)( _yscale * cosAngle );
		lm._41 = _x;
		lm._42 = _y;
		concatMatrix = Math::MultiplyMatrix( lm, parent->concatMatrix );
		displayObject->matrix->scaleX = lm._11;
		displayObject->matrix->scaleY = lm._22;
		displayObject->matrix->rotateSkew0 = lm._12;
		displayObject->matrix->rotateSkew1 = lm._21;
		displayObject->matrix->translateX = lm._41;
		displayObject->matrix->translateY = lm._42;

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
