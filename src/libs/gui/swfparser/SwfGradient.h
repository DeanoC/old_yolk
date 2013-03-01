// 
//  SwfGradient.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// ¡

#ifndef _SWFGRADIENT_H_
#define _SWFGRADIENT_H_

//#include <inttypes.h>
#include "SwfRGB.h"
#include "SwfStream.h"
namespace Swf
{
	// forward decl
	class SwfGradRecord;
	
	class SwfGradient
	{
	public:
        static const int MAX_GRADRECORDS = 15; // v1 = 8, v8 = 15
        enum SpreadMode
        {
            Pad = 0,
            Reflect = 1,
            Repeat = 2
        };
        enum InterpolationMode
        {
            NormalRGB = 0,
            LinearRGB = 1,
		};
		
		virtual ~SwfGradient (){};

        SpreadMode spreadMode;
        InterpolationMode interpolationMode;
        uint32_t numGradients;
        SwfGradRecord** gradientRecords;

		SwfRGBA SampleGradient(float ratio);
		
		static SwfGradient* Read(SwfStream& _stream, bool _shape3orMore);
		static SwfGradient* ReadMorph(SwfStream& _stream);

	private:
		/* data */
	};
	
	class SwfGradRecord
	{
	public:
		float   ratio;
		SwfRGBA colour;

		static SwfGradRecord* Read(SwfStream& _stream, bool _shape3orMore) {
			SwfGradRecord* record = CORE_NEW SwfGradRecord();

			record->ratio = ((float)_stream.readUInt8()) * (1.0f / 255.0f);
			record->colour = _shape3orMore ? SwfRGBA::ReadRGBA(_stream) : SwfRGBA::ReadRGB(_stream);
			return record;
		}
	};

	class SwfMorphGradRecord : public SwfGradRecord
	{
	public:
		float endRatio;
		SwfRGBA endColour;

		static SwfGradRecord* Read(SwfStream &_stream) {
			SwfMorphGradRecord* record = CORE_NEW SwfMorphGradRecord();	
			record->ratio = ((float)_stream.readUInt8()) * (1.0f / 255.0f);
			record->colour = SwfRGBA::ReadRGBA(_stream);
			record->endRatio = ((float)_stream.readUInt8()) * (1.0f / 255.0f);
			record->endColour = SwfRGBA::ReadRGBA(_stream);
			return record;
		}
	};
} /* Swf */


#endif /* _SWFGRADIENT_H_ */
