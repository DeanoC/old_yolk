// 
//  SwfMatrix.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfMatrix.h"
#include "SwfStream.h"
namespace Swf
{
	SwfMatrix* SwfMatrix::Read(SwfStream& _stream)
    {
        _stream.align();

        float scaleX = 1.0f, rotateSkew0 = 0.0f;
        float rotateSkew1 = 0.0f, scaleY = 1.0f;
        float translateX = 0.0f, translateY = 0.0f;

        bool hasScale = _stream.readFlag();
        if (hasScale)
        {
            int nScaleBits = (int)_stream.readUInt(5);
            scaleX = _stream.readInt(nScaleBits) / 65536.0f;
            scaleY = _stream.readInt(nScaleBits) / 65536.0f;
        }
        bool hasRotate = _stream.readFlag();
        if (hasRotate)
        {
            int nRotateBits = (int)_stream.readUInt(5);
            rotateSkew0 = _stream.readInt(nRotateBits) / 65536.0f;
            rotateSkew1 = _stream.readInt(nRotateBits) / 65536.0f;
        }
        int nTranslateBits = (int)_stream.readUInt(5);
        if (nTranslateBits > 0)
        {
            translateX = (float) _stream.readInt(nTranslateBits);
            translateY = (float) _stream.readInt(nTranslateBits);
        }

        return CORE_NEW SwfMatrix(scaleX, rotateSkew0, rotateSkew1, scaleY, translateX, translateY);
    }
    
} /* Swf */


