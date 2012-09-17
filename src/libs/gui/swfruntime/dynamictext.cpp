/*
 *  SwfRuntimeDynamicText.cpp
 *  Possessed
 *
 *  Created by Deano on 25/06/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "swfruntime.h"

#include "gui/SwfParser/SwfRect.h"
#include "gui/SwfParser/SwfTextRecord.h"
#include "gui/SwfParser/SwfFont.h"
#include "ActionScript/autogen/AsObject.h"
#include "frameitem.h"
#include "movieclip.h"
#include "font.h"
#include "player.h"
#include "dynamictext.h"


namespace Swf {
	DynamicText::DynamicText ( SwfDynamicText* _text) {
		dynText = _text;
		text = CORE_NEW SwfText( dynText->id );
		text->bounds = dynText->bounds;
	}
	
	
	// Does LEFT/CENTER/RIGHT alignment on the records in
	// m_text_glyph_records[], starting with
	// last_line_start_record and going through the end of
	// m_text_glyph_records.
	void DynamicText::align_line(SwfTextAlign _align, SwfTextRecord* _rec, float x) {
		float	extra_space = (dynText->bounds->Width() - dynText->rightMargin) - x;

		// extra_space may be < 0
		// assert(extra_space >= 0.0f);

		float	shift_right = 0.0f;

		if (_align == Left) {
			// Nothing to do; already aligned left.
			return;
		} else if (_align == Center) {
			// Distribute the space evenly on both sides.
			shift_right = extra_space / 2;
		} else if (_align == Right) {
			// Shift all the way to the right.
			shift_right = extra_space;
		}

		_rec->offset.x += (int)extra_space;
	}

	// Convert the characters in m_text into a series of
	// text_glyph_records to be rendered.
	void DynamicText::format_text(Player* _player, FrameItem* _parent) {
		Font* font = _player->getFont(dynText->fontId);

		std::vector<SwfTextRecord*> tempRecords;
		tempRecords.reserve(20);
		SwfTextRecord* rec = CORE_NEW SwfTextRecord();
		rec->fontID = dynText->fontId;
		rec->colour = dynText->fontColour;
		rec->offset.x = std::max(dynText->leftMargin + dynText->indent, 0);
		rec->offset.y = dynText->fontHeight;
		rec->textHeight = dynText->fontHeight;

		tempRecords.push_back(rec);

		SwfVec2Twip pos = rec->offset;
		int	leading = dynText->leading + font->font->fontLeading;
		float scale = dynText->fontHeight / 1024.0f;

		using namespace AutoGen;
		
		uint16_t code;
		std::string textstr;
		if( dynText->varName.empty() == false ) {
			AsObjectHandle textObj = _parent->parent->GetProperty( dynText->varName );
			if( textObj->Type() == APT_UNDEFINED ) {
				_parent->parent->SetProperty( dynText->varName, CORE_NEW AsObjectString( dynText->initialText ) );
				textObj = _parent->parent->GetProperty( dynText->varName );
			}
			textstr = textObj->ToString();
		} else {
			textstr = dynText->initialText;
		}

		std::vector<SwfGlyphEntry> tempGlyph;
		tempGlyph.reserve(160);

		for(size_t i=0;i < textstr.size();++i) {
			code = (uint16_t)textstr[i];
			if( code == 0xD || code == 0xA ) {
				pos.x = tempRecords[0]->offset.x;
				pos.y = pos.y + dynText->fontHeight + leading;
				rec->glyphEntries = CORE_NEW_ARRAY SwfGlyphEntry[tempGlyph.size()];
				rec->numGlyphEntries = tempGlyph.size();
				memcpy( rec->glyphEntries, &tempGlyph[0], sizeof(SwfGlyphEntry) * tempGlyph.size() );
				align_line(dynText->align, rec, (float) pos.x);

				// newline.
				rec = CORE_NEW SwfTextRecord();
				rec->fontID = dynText->fontId;
				rec->colour = dynText->fontColour;
				rec->textHeight = dynText->fontHeight;
				rec->offset = pos;
				tempGlyph.clear();
				tempRecords.push_back(rec);
				continue;
			}

			SwfGlyphEntry glyph;
			glyph.index = font->font->codeTable.find(code)->second;
			glyph.advance = (int)(font->font->fontAdvanceTable[glyph.index] * scale);
			tempGlyph.push_back(glyph);

			pos.x += glyph.advance;

			// word wrap
			if( dynText->wordWrap ){
				if (pos.x > (dynText->bounds->Width() - dynText->rightMargin) ) {
					pos.x = tempRecords[0]->offset.x;
					pos.y = pos.y + dynText->fontHeight + leading;
					rec->glyphEntries = CORE_NEW_ARRAY SwfGlyphEntry[tempGlyph.size()];
					rec->numGlyphEntries = tempGlyph.size();
					memcpy( rec->glyphEntries, &tempGlyph[0], sizeof(SwfGlyphEntry) * tempGlyph.size() );
					align_line(dynText->align, rec, (float) pos.x);

					// TODO word wrap here

					rec = CORE_NEW SwfTextRecord();
					rec->fontID = dynText->fontId;
					rec->colour = dynText->fontColour;
					rec->textHeight = dynText->fontHeight;
					rec->offset = pos;
					tempGlyph.clear();
					tempRecords.push_back(rec);
				}
			}
		}
		
		rec->glyphEntries = CORE_NEW_ARRAY SwfGlyphEntry[tempGlyph.size()];
		rec->numGlyphEntries = tempGlyph.size();
		memcpy( rec->glyphEntries, &tempGlyph[0], sizeof(SwfGlyphEntry) * tempGlyph.size() );
		align_line(dynText->align, rec, (float) pos.x);

		tempGlyph.clear();
		if( text->records != NULL){
			for( 	int i =0; i < text->numRecords;++i) {
				CORE_DELETE( text->records[i] );
			}
			
			CORE_DELETE_ARRAY text->records;
			text->numRecords = 0;
			text->records = NULL;
		}
		
		text->records = CORE_NEW_ARRAY SwfTextRecord*[tempRecords.size()];
		text->numRecords = tempRecords.size();
		for( 	std::vector<SwfTextRecord*>::iterator i = tempRecords.begin(); 
				i != tempRecords.end();
				++i ) {
			(*i)->offset.x += dynText->bounds->minX;
			(*i)->offset.y += dynText->bounds->minY;
			const int index = std::distance(tempRecords.begin(),i);
			text->records[index] = *i;
		}
	}
	
   void DynamicText::display(	Player* _player,
								Scene::RenderContext* _ctx,
								FrameItem* _parent,
								Math::Matrix4x4* _concatMatrix,
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph) {
       format_text(_player,_parent);
       Text::display( _player, _ctx, _parent, _concatMatrix, _colourTransform, _depth, _clipLayer, _morph );
   }
	
}