// 
//  SwfColourTransform.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#ifndef _SWFCOLOURTRANSFORM_H_
#define _SWFCOLOURTRANSFORM_H_

#include "SwfStream.h"

namespace Swf
{
	class SwfColourTransform
	{
	public:
		SwfColourTransform () {
			mul[0] = mul[1] = mul[2] = mul[3] = 1.0f;
			add[0] = add[1] = add[2] = add[3] = 0.0f;
		}
		virtual ~SwfColourTransform (){};

		float mul[4];
		float add[4];
 		static SwfColourTransform* Read(SwfStream& _stream, bool _withAlpha)
		{
			SwfColourTransform* trans = CORE_NEW SwfColourTransform();

			_stream.align();
			bool hasAddTerms = _stream.readFlag();
			bool hasMulTerms = _stream.readFlag();
			int nbits = (int)_stream.readUInt(4);
			if (hasMulTerms) {
				trans->mul[0] = ((float)_stream.readInt(nbits)) / 256.0f;
				trans->mul[1] = ((float)_stream.readInt(nbits)) / 256.0f;
				trans->mul[2] = ((float)_stream.readInt(nbits)) / 256.0f;
				if (_withAlpha) {
					trans->mul[3] = ((float)_stream.readInt(nbits)) / 256.0f;
				}
			}
			if (hasAddTerms) {
				trans->add[0] = ((float)_stream.readInt(nbits)) / 256.0f;
				trans->add[1] = ((float)_stream.readInt(nbits)) / 256.0f;
				trans->add[2] = ((float)_stream.readInt(nbits)) / 256.0f;
				if (_withAlpha) {
					trans->add[3] = ((float)_stream.readInt(nbits)) / 256.0f;
				}
			}
			return trans;
		}

		static const SwfColourTransform* const Identity() {
			static SwfColourTransform identity;
			return &identity;
		}

		static SwfColourTransform* Multiply(SwfColourTransform* _b,SwfColourTransform* _a) {
			SwfColourTransform* ret = CORE_NEW SwfColourTransform();
			for(int i=0;i < 4;++i) {
				ret->add[i] = _a->add[i] + (_a->mul[i] * _b->add[i]);
				ret->mul[i] = _a->mul[i] * _b->mul[i];
			}
			return ret;
		}

	private:
		/* data */
	};
} /* Swf */


#endif /* _SWFCOLOURTRANSFORM_H_ */

