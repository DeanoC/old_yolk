// 
//  SwfShapeRecord.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_SHAPERECORD_H)
#define _SWF_SHAPERECORD_H

#include "SwfStream.h"
#include "SwfVec2Twip.h"
namespace Swf{
	// forward decl
	class SwfFillStyle;
	class SwfLineStyle;
	
	class SwfShapeRecord
    {
	public:
		enum ShapeRecordType {
            End,
            StyleChange,
            StraightEdge,
            CurvedEdge
		};
		
        ShapeRecordType ShapeType() const {
                return type;
        }

		static SwfShapeRecord* Read(SwfStream& _stream, int _fillbits, int _linebits, int _shapeVer);

	protected:
        ShapeRecordType type;
        SwfShapeRecord(ShapeRecordType _type)
        {
            type = _type;
        }
	};
    
	class SwfEndShapeRecord : public SwfShapeRecord
    {
	public:
		SwfEndShapeRecord() :
            SwfShapeRecord(End)
        { 
        }
	};

    class SwfStyleChangeShapeRecord : public SwfShapeRecord
    {
	public:
		SwfStyleChangeShapeRecord() :
			fillStyle0(0),
			fillStyle1(0),
			lineStyle(0),
			moveDelta(0,0),
			fillStyleArray(0),
			lineStyleArray(0),
			numFillBits(0),
			numLineBits(0),
			newStylesFlag(false),
			lineStyleFlag(false),
			fillStyle0Flag(false),
			fillStyle1Flag(false),
			moveToFlag(false),
            SwfShapeRecord(StyleChange)
        { 
        }
        uint16_t fillStyle0;
        uint16_t fillStyle1;
        uint16_t lineStyle;
        SwfVec2Twip moveDelta;
		int numFillStyles;
		int numLineStyles;
        SwfFillStyle** fillStyleArray;
        SwfLineStyle** lineStyleArray;
        uint16_t numFillBits;
        uint16_t numLineBits;
        bool newStylesFlag;
        bool lineStyleFlag;
        bool fillStyle1Flag;
        bool fillStyle0Flag;
        bool moveToFlag;
	};

	class SwfStraightEdgeShapeRecord : public SwfShapeRecord {    
	public:
		SwfStraightEdgeShapeRecord(SwfVec2Twip _delta) :
            SwfShapeRecord(StraightEdge)
        {
            delta = _delta;
        }
        SwfVec2Twip delta;
        static SwfStraightEdgeShapeRecord* Read(SwfStream& _stream)
        {
            int numBits = 2 + (int)_stream.readUInt(4);
            SwfVec2Twip delta;
            bool generalFlag = _stream.readFlag();
            if (generalFlag)
            {
                int x = _stream.readInt(numBits);
                int y = _stream.readInt(numBits);
                delta = SwfVec2Twip(x,y);
            }
            else
            {
                bool isVertical = _stream.readFlag();
                if (isVertical)
                {
                    delta = SwfVec2Twip(0,_stream.readInt(numBits));
                } else
                {
                    delta = SwfVec2Twip(_stream.readInt(numBits),0);
                }
            }
            return CORE_NEW SwfStraightEdgeShapeRecord(delta);
        }
	};
	
	class SwfCurvedEdgeShapeRecord : public SwfShapeRecord 
	{
	public:
		SwfCurvedEdgeShapeRecord( SwfVec2Twip _controlDelta, SwfVec2Twip _anchorDelta ) :
		SwfShapeRecord(CurvedEdge) {
			controlDelta = _controlDelta;
			anchorDelta = _anchorDelta;
		}
		SwfVec2Twip controlDelta;
		SwfVec2Twip anchorDelta;

		static SwfCurvedEdgeShapeRecord* Read(SwfStream& _stream) {
			int numBits = 2 + (int)_stream.readUInt(4);
			SwfVec2Twip controlDelta;
			SwfVec2Twip anchorDelta;
			int cx = _stream.readInt(numBits);
			int cy = _stream.readInt(numBits);
			int ax = _stream.readInt(numBits);
			int ay = _stream.readInt(numBits);
			controlDelta = SwfVec2Twip(cx, cy);
			anchorDelta = SwfVec2Twip(ax, ay);

			return CORE_NEW SwfCurvedEdgeShapeRecord(controlDelta, anchorDelta);
		}
	};
}
#endif
