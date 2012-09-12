// 
//  SwfGradient.cpp
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "SwfGradient.h"
#include <math.h>


namespace Swf
{
	// TODO move
	static const float Epsilon = 1e-7f;
	
	// Return the color at the specified ratio into our gradient.
	SwfRGBA SwfGradient::SampleGradient(float ratio)
	{
		if (ratio < gradientRecords[0]->ratio)
			return gradientRecords[0]->colour;

		for (unsigned int i = 1; i < numGradients; i++) {
			if (gradientRecords[i]->ratio >= ratio) {
				SwfGradRecord* gr0 = gradientRecords[i - 1];
				SwfGradRecord* gr1 = gradientRecords[i];
				float	f = 0.0f;
				if ((gr1->ratio - gr0->ratio) > Epsilon) {
					f = (ratio - gr0->ratio) / (gr1->ratio - gr0->ratio);
				}
				if (f < 0.0f || f > 1.0f) {
					switch (spreadMode) {
					default:
					case Pad:
						f = std::max( 1.0f, std::min(0.0f,f));
					break;
					case Reflect:
						f = 1.0f - (f - (float)floor(f));
					break;
					case Repeat:
						f = f - (float)floor(f);
					break;
					}
				}
				return SwfRGBA::Lerp(gr0->colour, gr1->colour, 1.0f - f);
			}
		}
		return gradientRecords[numGradients - 1]->colour;
	}

	SwfGradient* SwfGradient::Read(SwfStream& _stream, bool _shape3orMore)
	{
	    _stream.align();
	    SwfGradient* gradient = CORE_NEW SwfGradient();

	    gradient->spreadMode = (SpreadMode)_stream.readUInt(2);
	    gradient->interpolationMode = (InterpolationMode)_stream.readUInt(2);
	    gradient->numGradients = _stream.readUInt(4);
	    gradient->gradientRecords = CORE_NEW SwfGradRecord*[gradient->numGradients];
	    for (uint32_t i = 0; i < gradient->numGradients; i++)
	    {
	        gradient->gradientRecords[i] = SwfGradRecord::Read(_stream, _shape3orMore);
	    }
	    return gradient;
	}

	SwfGradient* SwfGradient::ReadMorph(SwfStream& _stream)
	{
	    _stream.align();
	    SwfGradient* gradient = CORE_NEW SwfGradient();

	    gradient->spreadMode = Pad;
	    gradient->interpolationMode = NormalRGB;
	    gradient->numGradients = _stream.readUInt(4);
	    gradient->gradientRecords = (SwfGradRecord**)CORE_NEW SwfMorphGradRecord*[gradient->numGradients];
	    for (uint32_t i = 0; i < gradient->numGradients; i++)
	    {
	        gradient->gradientRecords[i] = SwfMorphGradRecord::Read(_stream);
	    }
	    return gradient;
	}
    
} /* Swf */
