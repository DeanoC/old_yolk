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
	SwfButtonObject* SwfButtonObject::Read( SwfStream& _stream, int _length, int _version ) {
		
		long marker = _stream.marker();
		
		uint16_t id = _stream.readUInt16();

		SwfButtonObject* button = CORE_NEW SwfButtonObject(id);
		
		bool trackAsMenu = false;
		uint16_t actionOffset = 0;
		if( _version == 2 ) {
			// skip reserved
			_stream.readUInt(7);
			trackAsMenu = _stream.readFlag();
			marker = _stream.marker();
			actionOffset = _stream.readUInt16();
		}
		bool endRecord = false;
		do 
		{
			_stream.readUInt(2);
			bool buttonHasBlendMode = _stream.readFlag();
			bool buttonHasFilterList = _stream.readFlag();
			bool buttonStateHitTest = _stream.readFlag();
			bool buttonStateDown = _stream.readFlag();
			bool buttonStateOver = _stream.readFlag();
			bool buttonStateUp = _stream.readFlag();
			endRecord = buttonStateHitTest | buttonStateDown | buttonStateOver | buttonStateUp;
			if( endRecord ) {
				uint16_t characterId = _stream.readUInt16();
				uint16_t placeDepth  = _stream.readUInt16();
				SwfMatrix* matrix = SwfMatrix::Read( _stream );
				SwfColourTransform* cxform;
				if( _version == 2) {
					cxform = SwfColourTransform::Read( _stream, true );
					if( buttonHasFilterList ) {
						
					}
					if( buttonHasBlendMode ) {
						
					}
				} else {
					cxform = CORE_NEW SwfColourTransform();
				}
			}			
		} while( endRecord == false );

		// version one only had one action press and release
		if( _version == 1 ) {
			_length -= (_stream.marker() - marker);
		
			SwfActionByteCode* actionScript = CORE_NEW SwfActionByteCode();
			actionScript->byteCode = CORE_NEW_ARRAY uint8_t[_length];
			actionScript->lengthInBytes = _length;
			if( true /*fileVersion < 7*/ ) {
				actionScript->isCaseSensitive = false;
			} else {
				actionScript->isCaseSensitive = true;
			}
			_stream.readBytes(actionScript->byteCode, actionScript->lengthInBytes);
		} else {
			while( actionOffset != 0 ) {
				// version 2 has many...
				_stream.setToMarker( marker + actionOffset );
				marker = _stream.marker();
				actionOffset = _stream.readUInt16();
				bool idleToOverDown = _stream.readFlag();
				bool outDownToIdle = _stream.readFlag();
				bool outDownToOverDown = _stream.readFlag();
				bool overDownToOutDown = _stream.readFlag();
				bool overDownToOverUp = _stream.readFlag();
				bool overUpToOverDown = _stream.readFlag();
				bool overUpToIdle = _stream.readFlag();
				bool idleToOverUp  = _stream.readFlag();
				bool overDownToIdle = _stream.readFlag();
				
				uint8_t keyCode = _stream.readUInt(7);
				
				uint8_t temp[1024];
				uint16_t byteCount = 0;
				uint8_t b = 0;
				while( b = _stream.readUInt8() ) {
					if( byteCount < 1024 ) {
						temp[byteCount++] = b;
					}
				}
				SwfActionByteCode* actionScript = CORE_NEW SwfActionByteCode();
				actionScript->byteCode = CORE_NEW_ARRAY uint8_t[byteCount+1];
				actionScript->lengthInBytes = byteCount;
				if( true /*fileVersion < 7*/ ) {
					actionScript->isCaseSensitive = false;
				} else {
					actionScript->isCaseSensitive = true;
				}
				memcpy( actionScript->byteCode, temp, byteCount );
				actionScript->byteCode[ byteCount ] = 0;
				
			}
			
		}
		
		return button;
	}
} /* Swf */ 
