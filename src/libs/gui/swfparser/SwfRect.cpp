// 
//  SwfRect.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfRect.h"
#include "SwfStream.h"

namespace Swf{
	SwfRect* SwfRect::Read(SwfStream& _stream)
	{
        _stream.align();
        int nbits = (int)_stream.readUInt(5);
        int minx = _stream.readInt(nbits);
        int maxx = _stream.readInt(nbits);
        int miny = _stream.readInt(nbits);
        int maxy = _stream.readInt(nbits);

        return CORE_NEW SwfRect(minx, miny, maxx, maxy);
	}

}