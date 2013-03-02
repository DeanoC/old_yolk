// 
//  SwfClipActions.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfClipActions.h"
#include "SwfStream.h"
#include <list>
namespace Swf
{
	SwfClipActions* SwfClipActions::Read(SwfStream& _stream)
    {
		uint16_t reserved = _stream.readUInt16();
		UNUSED(reserved);

		ClipEventFlags flags;
		if (_stream.version >= 6)
		{
		   flags = (ClipEventFlags)_stream.readUInt32();
		}
		else
		{
		   flags = (ClipEventFlags)_stream.readUInt16();
		}
		std::list<SwfClipActionRecord*> tempClipActions;
		SwfClipActionRecord* lastAction = NULL;
		do
		{
		   lastAction = SwfClipActionRecord::Read(_stream);
		   if (lastAction != NULL)
		   {
		       tempClipActions.push_back(lastAction);
		   }
		} while (lastAction != NULL);

		SwfClipActions* clipAction = CORE_NEW SwfClipActions();
		clipAction->allEventFlags = flags;
		clipAction->clipActionRecords = CORE_NEW SwfClipActionRecord*[tempClipActions.size()];
		clipAction->numClipActions = tempClipActions.size();
		for( std::list<SwfClipActionRecord*>::iterator i = tempClipActions.begin(); i != tempClipActions.end();++i)
		{
			clipAction->clipActionRecords[std::distance(tempClipActions.begin(),i)] = *i;
		}
		return clipAction;
    }
	
	SwfClipActions::~SwfClipActions() {
		for( auto i = 0 ; i < numClipActions; ++i ) {
			CORE_DELETE( clipActionRecords[i] );
		}
		CORE_DELETE_ARRAY( clipActionRecords );
	}

    SwfClipActionRecord* SwfClipActionRecord::Read(SwfStream& _stream)
    {
			ClipEventFlags flags;
			if (_stream.version >= 6) {
				flags = (ClipEventFlags)_stream.readUInt32();
			}
			else {
				flags = (ClipEventFlags)_stream.readUInt16();
			}
			// end readInt
			if (flags == 0) {
				return NULL;
			}
			SwfClipActionRecord* car = CORE_NEW SwfClipActionRecord();
			car->eventFlags = flags;

			uint32_t nextOffset = _stream.readUInt32();
			uint64_t nextClipRecord = _stream.marker() + nextOffset;

			if ((flags & KeyPress) == KeyPress) {
				car->keyCode = _stream.readUInt8();
			} else {
				car->keyCode = 0;
			}
			int numBytes = (int)(nextClipRecord - _stream.marker());
			car->byteCode = CORE_NEW uint8_t[numBytes];
			_stream.readBytes(car->byteCode, numBytes);

			return car;
    }
	SwfClipActionRecord::~SwfClipActionRecord() {
		CORE_DELETE_ARRAY( byteCode );
	}
    
} /* Swf */
