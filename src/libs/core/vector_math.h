//!-----------------------------------------------------
//!
//! \file vector_maths.h
//! contains the maths namespace stuff
//!
//!-----------------------------------------------------
#pragma once

#ifndef CORE_VECTOR_MATHS_H_
#define CORE_VECTOR_MATHS_H_


#include "core/core.h"
#include <math.h>
#include "mathconsts.h"


#if PLATFORM == WINDOWS

#if defined( _MSC_VER ) && !defined( MSC_FLOAT_CoreIS_HACK ) 
#define MSC_FLOAT_CoreIS_HACK
namespace Core {
inline bool isnan( float a ) { return _isnan( a ) != 0; }
inline bool isnan( double a ) { return _isnan( a ) != 0; }
inline bool isfinite( float a ) { return _finite( a ) != 0; }
inline bool isfinite( double a ) { return _finite( a ) != 0; }
inline bool isnormal( float a ) 
{ 
	switch( _fpclass( a ) )
	{
	case _FPCLASS_NN:
	case _FPCLASS_NZ:
	case _FPCLASS_PZ:
	case _FPCLASS_PN:
		return true;

	default:
		return false;
	}
}
inline bool isnormal( double a ) 
{ 
	switch( _fpclass( a ) )
	{
	case _FPCLASS_NN:
	case _FPCLASS_NZ:
	case _FPCLASS_PZ:
	case _FPCLASS_PN:
		return true;

	default:
		return false;
	}
}
} // namespace Core
#endif // def _MSC_VER

#if defined( USE_DX11 )
#	include "vector_math_dx10.h"
#else
#endif //end USE_DX11

#else

namespace Core {
	using std::isnan;
	using std::isfinite;
	using std::isnormal;
}

#endif // end WIN32

// if there is no platform specific version fall back to a standard C implementation
#if !defined(PLATFORM_SPECIFIC_VECTOR_MATH_DEFINED)
#include "vector_math_c.h"
#endif

// function shared by all maths implementaiton
namespace Math {
	//! Should be called at least once to do any maths initialisation thats needed (tables being built etc.)
	void Init( void );
	//! Should be called on exit to shut down any maths stuff
	void Shutdown( void );

	//! A not very fast or stable matrix determinant there may be better version
	float C_Determinant( const Matrix4x4& mat );
	//! A not very fast or stable matrix adjoint there may be better version
	Matrix4x4 C_Adjoint( const Matrix4x4& mat );
	//! A not very fast or stable matrix invert there may be better version
	Matrix4x4 C_Invert( const Matrix4x4& mat );

	CALL inline uint64_t MortonCurve( const Math::Vector3& coord, const Math::Vector3& max ) {
		// make temp be coord in a normalised 0 to 1 range
		Math::Vector3 temp = Math::ComponentMultiply( Math::Reciprocal(max*2.0f), coord ) + 
														Math::Vector3(0.5f,0.5f,0.5f);
		temp = Math::Clamp( temp, Math::Vector3(0,0,0), Math::Vector3(1,1,1) );

		const int NUM_BITS = 21;
		temp = temp * (float)(1 << NUM_BITS); // each coord component gets 21 bits 
		uint64_t x = (uint32_t)temp.x;   // Interleave bits of x and y and z
		uint64_t y = (uint32_t)temp.y;   
		uint64_t z = (uint32_t)temp.z;
		uint64_t r = 0; // r gets the resulting 63-bit Morton Number.

		// unroll for more speed...
		for (int64_t i = 0; i < NUM_BITS; i++) {
			uint64_t anders = (1UL << i);
			r |=	((x & anders) >> i)	<< ((i*3)+0)	| 
					((y & anders) >> i) << ((i*3)+1)	|
					((z & anders) >> i) << ((i*3)+2);
		}
		return r;
	}

	CALL inline  bool IsFinite( const Vector2& a ) {
		return( Core::isfinite(a.x) && Core::isfinite(a.y) );
	}
	CALL inline  bool IsFinite( const Vector3& a ) {
		return( Core::isfinite(a.x) && Core::isfinite(a.y) && Core::isfinite(a.z) );
	}
	CALL inline  bool IsFinite( const Vector4& a ) {
		return( Core::isfinite(a.x) && Core::isfinite(a.y) && Core::isfinite(a.z) && Core::isfinite(a.w));
	}


}

// just because of the ordering of class we need to have this here as it fall back to the C_Invert function
#if !defined(PLATFORM_SPECIFIC_VECTOR_MATH_DEFINED)
namespace Math {
	inline Matrix4x4 InverseMatrix( const Matrix4x4& mat ){ return C_Invert(mat); }
}
#endif


#endif

