/*
 *  SwfButton.cpp
 *  SwfPreview
 *
 *  Created by Deano on 13/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "swfparser.h"
#include "SwfMatrix.h"
#include "SwfColourTransform.h"
#include "SwfActionByteCode.h"
#include "SwfButton.h"

namespace Swf
{
	SwfButton* SwfButton::Read( SwfStream& _stream, int _length, int _version ) {
		
		uint64_t marker = _stream.marker();
		uint64_t startMarker = marker;
		
		uint16_t id = _stream.readUInt16();

		SwfButton* button = CORE_NEW SwfButton(id);
		
		bool trackAsMenu = false;
		uint16_t actionOffset = 0;
		if( _version == 2 ) {
			// skip reserved
			_stream.readUInt(7);
			trackAsMenu = _stream.readFlag();
			marker = _stream.marker();
			actionOffset = _stream.readUInt16();
		}

		std::vector< SwfButtonRecord > records;
		bool endRecord = false;
		do 
		{
			SwfButtonRecord rec;
			_stream.align();
			_stream.readUInt(2);
			rec.buttonHasBlendMode = _stream.readFlag();
			rec.buttonHasFilterList = _stream.readFlag();
			rec.buttonStateHitTest = _stream.readFlag();
			rec.buttonStateDown = _stream.readFlag();
			rec.buttonStateOver = _stream.readFlag();
			rec.buttonStateUp = _stream.readFlag();
			endRecord = rec.buttonStateHitTest | rec.buttonStateDown | rec.buttonStateOver | rec.buttonStateUp;
			if( endRecord ) {
				rec.characterId = _stream.readUInt16();
				rec.placeDepth  = _stream.readUInt16();
				rec.matrix = SwfMatrix::Read( _stream );
				if( _version == 2) {
					rec.cxform = SwfColourTransform::Read( _stream, true );
					if( rec.buttonHasFilterList ) {
						
					}
					if( rec.buttonHasBlendMode ) {
						
					}
				} else {
					rec.cxform = CORE_NEW SwfColourTransform();
				}
				records.push_back( rec );
			}
		} while( endRecord == true );

		std::vector<SwfButtonCondAction> condActions;
		SwfButtonCondAction bca;
		// version one only had one action press and release
		if( _version == 1 ) {
			// TODO 64 bit files
			_length -= (int)(_stream.marker() - marker);
		
			memset( &bca, 0, sizeof( bca ) );
			SwfActionByteCode* actionScript = CORE_NEW SwfActionByteCode();
			actionScript->byteCode = CORE_NEW_ARRAY uint8_t[_length];
			actionScript->lengthInBytes = _length;
			if( true /*fileVersion < 7*/ ) {
				actionScript->isCaseSensitive = false;
			} else {
				actionScript->isCaseSensitive = true;
			}
			_stream.readBytes(actionScript->byteCode, actionScript->lengthInBytes);
			bca.overUpToOverDown = true;
			bca.actionScript = actionScript;
			condActions.push_back( bca );

		} else {
			while( actionOffset != 0 ) {
				// version 2 has many...
				_stream.setToMarker( marker + actionOffset );
				marker = _stream.marker();
				actionOffset = _stream.readUInt16();
				bca.idleToOverDown = _stream.readFlag();
				bca.outDownToIdle = _stream.readFlag();
				bca.outDownToOverDown = _stream.readFlag();
				bca.overDownToOutDown = _stream.readFlag();
				bca.overDownToOverUp = _stream.readFlag();
				bca.overUpToOverDown = _stream.readFlag();
				bca.overUpToIdle = _stream.readFlag();
				bca.idleToOverUp  = _stream.readFlag();
				bca.keyCode = _stream.readUInt(7);
				bca.overDownToIdle = _stream.readFlag();
	
				auto len = actionOffset - (marker - _stream.marker());
				if( actionOffset == 0 ) {
					len = (_length + startMarker)- _stream.marker();
				}
				std::vector<uint8_t> temp;
				temp.reserve( (uint32_t)len + 1 );

				for( int i = 0; i < len; ++i ) {
					temp.push_back( _stream.readUInt8() );
				}
			
				SwfActionByteCode* actionScript = CORE_NEW SwfActionByteCode();
				actionScript->byteCode = CORE_NEW_ARRAY uint8_t[ temp.size() + 1 ];
				actionScript->lengthInBytes = (uint32_t)temp.size();
				if( true /*fileVersion < 7*/ ) {
					actionScript->isCaseSensitive = false;
				} else {
					actionScript->isCaseSensitive = true;
				}
				memcpy( actionScript->byteCode, &temp[0], temp.size() );
				actionScript->byteCode[ temp.size() ] = 0;
				bca.actionScript = actionScript;
				condActions.push_back( bca );
			}
			
		}

		button->records = CORE_NEW_ARRAY SwfButtonRecord[ records.size() ];
		memcpy( button->records, &records[0], sizeof( SwfButtonRecord ) * records.size() );
		button->numRecords = (int)records.size();

		button->condActions = CORE_NEW_ARRAY SwfButtonCondAction[ condActions.size() ];
		memcpy( button->condActions, &condActions[0], sizeof( SwfButtonCondAction ) * condActions.size() );
		button->numCondActions = (int)condActions.size();
		
		return button;
	}
	SwfButton::~SwfButton() {
		CORE_DELETE_ARRAY( records );
		CORE_DELETE_ARRAY( condActions );
	}
} /* Swf */ 
