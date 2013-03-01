//!-----------------------------------------------------
//!
//! \file colour.h
//! A class representing a colour, currently
//! the usual spectral triplet and alpha are stored in HDR
//!
//!
//!-----------------------------------------------------
#pragma once

#ifndef YOLK_CORE_COLOUR_H_
#define YOLK_CORE_COLOUR_H_

#ifndef CORE_VECTOR_MATHS_H_
#	include "vector_math.h"
#endif

namespace Core
{
class RGBAColour;

//! Colour interface
class Colour {
public:
	//! intended to convert from internal colour space to RGBA
	virtual const RGBAColour& getRGBAColour() const = 0;
	virtual void setRGBAColour( const RGBAColour& colour ) = 0;

	// common short cut same as RGBAColour::packARGV( getRGBAColour() )
	uint32_t packARGB() const;
};

class RGBAColour : public Colour, public Math::Vector4 {
public:
	RGBAColour() {}

	explicit RGBAColour( const Math::Vector4& colour ) :
		Math::Vector4( colour ) {}

	RGBAColour( float r, float g, float b, float a ) :
		Math::Vector4( r, g, b, a ) {}

	const Math::Vector4& getAsVector() const { return *this; }

	/* Inherited */
	//! intended to convert from internal colour space to RGBA
	virtual const RGBAColour& getRGBAColour() const { return *this; }
	virtual void setRGBAColour( const RGBAColour& colour ) { *this = colour; }


	/* Static */
	//! reads a PC endian-ness specific ARGB colour
	static RGBAColour unpackARGB( uint32_t uintCol ) {
		const float coeff = 1.0f / 255.0f;
		return RGBAColour(	float( (uintCol >> 16) & 0xFF ) * coeff,
							float( (uintCol >>  8) & 0xFF ) * coeff,
							float( (uintCol >>  0) & 0xFF ) * coeff,
							float( (uintCol >> 24) & 0xFF ) * coeff );
	}

	//! returns a PC endian-ness specific ARGB colour
	static uint32_t packARGB( const RGBAColour& colour ) {
		uint32_t R = uint32_t( Math::Clamp( colour.x * 255.0f, 0.0f, 255.0f ) );
		uint32_t G = uint32_t( Math::Clamp( colour.y * 255.0f, 0.0f, 255.0f ) );
		uint32_t B = uint32_t( Math::Clamp( colour.z * 255.0f, 0.0f, 255.0f ) );
		uint32_t A = uint32_t( Math::Clamp( colour.w * 255.0f, 0.0f, 255.0f ) );

		return uint32_t( (A << 24) | (R << 16) | (G << 8) | B );
	}

};

inline uint32_t Colour::packARGB() const {
	return RGBAColour::packARGB( this->getRGBAColour() );
}

}	//namespace Core


#endif
