// 
//  SwfDisplayObject.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfDisplayObject.h"
#include "SwfStream.h"
#include "SwfMatrix.h"
#include "SwfColourTransform.h"
#include "SwfClipActions.h"
namespace Swf
{
	SwfRemoveObject* SwfRemoveObject::Read(SwfStream& _stream, int _removeVer) {
		SwfRemoveObject* obj = CORE_NEW SwfRemoveObject();

		_stream.align();
		if (_removeVer == 1) {
			// remove object gave both a charcter id and and a depth
			// but as you can only have 1 character per depth this
			// was dropped, so just throwing away... I expect really
			// old swf allow multiple character per depth, but I 
			// don't really support this, so...
			uint16_t charId = _stream.readUInt16();
			UNUSED( charId );
		}
		obj->depth = _stream.readUInt16();
		return obj;
	}

	SwfDisplayObject* SwfDisplayObject::Read(SwfStream& _stream, int _swfVersion) {
		SwfDisplayObject* obj = CORE_NEW SwfDisplayObject();

		if (_swfVersion == 1) {
			obj->id = _stream.readUInt16();
			obj->depth = _stream.readUInt16();
			obj->matrix = SwfMatrix::Read(_stream);
			// an optional colour transform is here?? detect base on size
			// of tag? erm not sure for now ignore as this is only for
			// ancient Swf
			return obj;
		}

		_stream.align();
		bool hasClipActions = _stream.readFlag();
		obj->hasClipDepth = _stream.readFlag();
		bool hasName = _stream.readFlag();
		obj->hasMorphRatio = _stream.readFlag();
		obj->hasColourTransform = _stream.readFlag();
		obj->hasMatrix = _stream.readFlag();
		bool hasCharacterId = _stream.readFlag();
		bool hasMove = _stream.readFlag();
		UNUSED(hasMove);

		obj->depth = _stream.readUInt16();
		if (hasCharacterId) {
			obj->id = _stream.readUInt16();
		}
		if (obj->hasMatrix) {
			obj->matrix = SwfMatrix::Read(_stream);
		} else {
			// identity matrix
			obj->matrix = CORE_NEW SwfMatrix();
		}
		if (obj->hasColourTransform) {
			obj->cxform = SwfColourTransform::Read(_stream, true);
		} else {
			obj->cxform = CORE_NEW SwfColourTransform();
		}
		if (obj->hasMorphRatio) {
			obj->morphRatio = ((float)_stream.readUInt16()) / 65535.0f;
		}
		if (hasName) {
			obj->name = _stream.readString();
		}
		if (obj->hasClipDepth) {
			obj->clipDepth = _stream.readUInt16();
		}
		if (hasClipActions) {
			obj->clipActions = SwfClipActions::Read(_stream);
		}
		return obj;
	}    
} /* Swf */
