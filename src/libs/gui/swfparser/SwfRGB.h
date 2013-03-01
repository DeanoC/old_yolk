// 
//  SwfRGB.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_RGB_H)
#define _SWF_RGB_H

//#include <inttypes.h>

namespace Swf{
	//forward decl
	class SwfStream;

	/// SwfRGBA by default SWF use sRGB colours
	struct SwfRGBA {
		SwfRGBA(){
			r = g = b = a = 0.0f;
		}

		explicit SwfRGBA(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) {
			r = _r * (1.0f / 255.0f);
			g = _g * (1.0f / 255.0f);
			b = _b * (1.0f / 255.0f);
			a = _a * (1.0f / 255.0f);
		}

		explicit SwfRGBA(float _r, float _g, float _b, float _a) {
			r = _r;
			g = _g;
			b = _b;
			a = _a;
		}

		uint32_t ToPackedRGBA();
		uint32_t ToPackedARGB();
		uint32_t ToPackedABGR();
		
		static SwfRGBA Lerp(const SwfRGBA& _a, const SwfRGBA& _b, float t);
		static SwfRGBA LinearLerp( const SwfRGBA& _a, const SwfRGBA& _b, float t );
		
		static SwfRGBA ReadRGB(SwfStream& _stream);
		static SwfRGBA ReadRGBA(SwfStream& _stream);
		static SwfRGBA ReadARGB(SwfStream& _stream);

		float getRed() const { return r; }
		float getGreen() const { return g; }
		float getBlue() const { return b; }
		float getAlpha() const { return a; }
		
		float getLinearRed() const;
		float getLinearGreen() const;
		float getLinearBlue() const;

	private:
		float r;
		float g;
		float b;
		float a;
	};
}
#endif