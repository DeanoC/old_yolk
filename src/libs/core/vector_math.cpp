//!-----------------------------------------------------
//!
//! \file vector_maths.cpp
//! some maths functions
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "vector_math.h"

namespace
{
	float DetInternal(	float a1, float a2, float a3,
						float b1, float b2, float b3,
						float c1, float c2, float c3 )
	{
		float ret =	a1 * (b2 * c3 - b3 * c2) -
					b1 * (a2 * c3 - a3 * c2) +
					c1 * (a2 * b3 - a3 * b2);

		return ret;
	}

}

namespace Math
{
	//!-----------------------------------------------------
	//!
	//! Initialisation of the maths library
	//!
	//!-----------------------------------------------------
	void Init( void )
	{
	}
	//!-----------------------------------------------------
	//!
	//! Shutdowns the maths libray
	//!
	//!-----------------------------------------------------
	void Shutdown( void )
	{
	}


	float C_Determinant( const Matrix4x4& mat )
	{
		float a1,a2,a3,a4;
		float b1,b2,b3,b4;
		float c1,c2,c3,c4;
		float d1,d2,d3,d4;

		a1 = mat(0,0); b1 = mat(0,1); c1 = mat(0,2); d1 = mat(0,3);
		a2 = mat(1,0); b2 = mat(1,1); c2 = mat(1,2); d2 = mat(1,3);
		a3 = mat(2,0); b3 = mat(2,1); c3 = mat(2,2); d3 = mat(2,3);
		a4 = mat(3,0); b4 = mat(3,1); c4 = mat(3,2); d4 = mat(3,3);

		float ret = a1 * DetInternal(b2,b3,b4,c2,c3,c4,d2,d3,d4) - 
					b1 * DetInternal(a2,a3,a4,c2,c3,c4,d2,d3,d4) + 
					c1 * DetInternal(a2,a3,a4,b2,b3,b4,d2,d3,d4) - 
					d1 * DetInternal(a2,a3,a4,b2,b3,b4,c2,c3,c4);
		
		return ret;
	}

	Matrix4x4 C_Adjoint( const Matrix4x4& mat )
	{
		float a1,a2,a3,a4;
		float b1,b2,b3,b4;
		float c1,c2,c3,c4;
		float d1,d2,d3,d4;

		a1 = mat(0,0); b1 = mat(0,1); c1 = mat(0,2); d1 = mat(0,3);
		a2 = mat(1,0); b2 = mat(1,1); c2 = mat(1,2); d2 = mat(1,3);
		a3 = mat(2,0); b3 = mat(2,1); c3 = mat(2,2); d3 = mat(2,3);
		a4 = mat(3,0); b4 = mat(3,1); c4 = mat(3,2); d4 = mat(3,3);

		Matrix4x4 ret;
		ret(0,0) =  DetInternal(b2,b3,b4,c2,c3,c4,d2,d3,d4);
		ret(1,0) = -DetInternal(a2,a3,a4,c2,c3,c4,d2,d3,d4);
		ret(2,0) =  DetInternal(a2,a3,a4,b2,b3,b4,d2,d3,d4);
		ret(3,0) = -DetInternal(a2,a3,a4,b2,b3,b4,c2,c3,c4);

		ret(0,1) = -DetInternal(b1,b3,b4,c1,c3,c4,d1,d3,d4);
		ret(1,1) =  DetInternal(a1,a3,a4,c1,c3,c4,d1,d3,d4);
		ret(2,1) = -DetInternal(a1,a3,a4,b1,b3,b4,d1,d3,d4);
		ret(3,1) =  DetInternal(a1,a3,a4,b1,b3,b4,c1,c3,c4);

		ret(0,2) =  DetInternal(b1,b2,b4,c1,c2,c4,d1,d2,d4);
		ret(1,2) = -DetInternal(a1,a2,a4,c1,c2,c4,d1,d2,d4);
		ret(2,2) =  DetInternal(a1,a2,a4,b1,b2,b4,d1,d2,d4);
		ret(3,2) = -DetInternal(a1,a2,a4,b1,b2,b4,c1,c2,c4);

		ret(0,3) = -DetInternal(b1,b2,b3,c1,c2,c3,d1,d2,d3);
		ret(1,3) =  DetInternal(a1,a2,a3,c1,c2,c3,d1,d2,d3);
		ret(2,3) = -DetInternal(a1,a2,a3,b1,b2,b3,d1,d2,d3);
		ret(3,3) =  DetInternal(a1,a2,a3,b1,b2,b3,c1,c2,c3);

		return ret;

	}

	Matrix4x4 C_Invert( const Matrix4x4& mat )
	{
		float det = C_Determinant(mat);
		Matrix4x4 ret;

		if( fabsf(det) < 0.000001f )
			ret = IdentityMatrix();
		else
		{
			ret = C_Adjoint(mat);
			ret = ret * (1.f / det);
		}

		return ret;
	}


};
