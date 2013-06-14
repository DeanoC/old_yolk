/*
 *  SwfRuntimeText.cpp
 *  Possessed
 *
 *  Created by Deano on 24/06/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "swfruntime.h"
#include "gui/swfparser/SwfText.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "gui/swfparser/SwfTextRecord.h"
#include "font.h"
#include "player.h"
#include "shape.h"
#include "utils.h"
#include "text.h"

namespace Swf {
void Text::display(		const Player* _player,
						Scene::RenderContext* _ctx, 
						FrameItem* _parent,
						Math::Matrix4x4* _concatMatrix, 
						SwfColourTransform* _colourTransform, 
						uint16_t _depth, bool _clipLayer, float _morph) {
	Font* font = NULL;
	Math::Matrix4x4 charamat;
	if(text->matrix != NULL ) {
		charamat = Math::MultiplyMatrix(Convert(text->matrix), *_concatMatrix);
	} else {
		charamat = *_concatMatrix;
	}

	for(int i = 0; i < text->numRecords; ++i) {
		SwfTextRecord* rec = text->records[i];
		// swish seems to forget rec.hasFont sometimes
		if (rec->hasFont || font == NULL) {
			font = _player->getFont( rec->fontID );
		}
		
		// force rec colour into each fill in the shape
		SwfColourTransform fakeCT;
		fakeCT.mul[0] = fakeCT.mul[1] = fakeCT.mul[2] = fakeCT.mul[3] = 0;
		fakeCT.add[0] = rec->colour.getLinearRed() * _colourTransform->mul[0] + _colourTransform->add[0];
		fakeCT.add[1] = rec->colour.getLinearGreen() * _colourTransform->mul[1] + _colourTransform->add[1];
		fakeCT.add[2] = rec->colour.getLinearBlue() * _colourTransform->mul[2] + _colourTransform->add[2];
		fakeCT.add[3] = rec->colour.getAlpha() * _colourTransform->mul[3] + _colourTransform->add[3];

		SwfVec2Twip offset = rec->offset;
		float scale = rec->textHeight / 1024.0f;
		for(int j = 0; j < rec->numGlyphEntries; ++j) {
			SwfGlyphEntry* glyph = &rec->glyphEntries[j];
			Shape* shape = font->glyphShapes[glyph->index];

			Math::Matrix4x4 nmat = Math::Matrix4x4( 	scale, 				0, 	  					0,				0,
     														0, 			scale, 	  					0,				0,
															0,				0,						1,				0,
															(float)offset.x, 	(float)offset.y, 	0,				1 );
			nmat = Math::MultiplyMatrix( nmat, charamat );
			shape->display( _player, _ctx, _parent, &nmat, &fakeCT, _depth, _clipLayer, _morph );
			offset.x = offset.x + glyph->advance;			
		}
	}	
}
	
}