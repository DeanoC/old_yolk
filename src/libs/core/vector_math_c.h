//!-----------------------------------------------------
//!
//! \file vector_maths_c.h
//! contains the maths stuff in the default C impl
//! Matrices are row-major, Left Handed
//!
//!-----------------------------------------------------

#pragma once

#ifndef WEIRD_CORE_VECTOR_MATHS_C_H
#define WEIRD_CORE_VECTOR_MATHS_C_H


//! Maths functions live in here
namespace Math {
	class Vector2 {
	public:
	    CALL Vector2() {};
		CALL Vector2( const float * fArr) { CORE_ASSERT(fArr); x = fArr[0]; y = fArr[1]; };
		CALL Vector2( float _x, float _y ) { x = _x; y = _y; };

	    // casting
		CALL operator float* () { return &x; }
		CALL operator const float* () const { return &x; }

		// assignment operators
		CALL Vector2& operator += ( const Vector2& rhs ){ x += rhs.x; y += rhs.y; return *this; }
		CALL Vector2& operator -= ( const Vector2& rhs ){ x -= rhs.x; y -= rhs.y; return *this; };
		CALL Vector2& operator *= ( float rhs ){ x *= rhs; y *= rhs; return *this; };
		CALL Vector2& operator /= ( float rhs ){ y /= rhs; y /= rhs; return *this; };

		// unary operators
		CALL Vector2 operator + () const { return Vector2(x,y); }
		CALL Vector2 operator - () const { return Vector2(-x,-y); }

		// binary operators
		CALL Vector2 operator + ( const Vector2& rhs) const { return Vector2(x+rhs.x, y+rhs.y); }
		CALL Vector2 operator - ( const Vector2& rhs) const { return Vector2(x-rhs.x, y-rhs.y); };
		CALL Vector2 operator * ( float rhs) const { return Vector2(x*rhs, y*rhs); };
		CALL Vector2 operator / ( float rhs) const { return Vector2(x/rhs, y/rhs); };

		CALL friend Vector2 operator * ( float lhs, const Vector2& rhs) { return Vector2( lhs*rhs.x, lhs*rhs.y); };

		CALL bool operator == ( const Vector2& rhs) const { return x == rhs.x && y == rhs.y; }
		CALL bool operator != ( const Vector2& rhs) const { return x != rhs.x || y != rhs.y; };
    
		float x, y;
	};

	class Vector3
	{
	public:
	    CALL Vector3() {};
		CALL Vector3( const float * fArr) { CORE_ASSERT(fArr); x = fArr[0]; y = fArr[1]; z = fArr[2]; };
		CALL Vector3( float _x, float _y, float _z ) { x = _x; y = _y; z = _z; };

	    // casting
		CALL operator float* () { return &x; }
		CALL operator const float* () const { return &x; }

		// assignment operators
		CALL Vector3& operator += ( const Vector3& rhs ){ x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
		CALL Vector3& operator -= ( const Vector3& rhs ){ x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; };
		CALL Vector3& operator *= ( float rhs ){ x *= rhs; y *= rhs; z *= rhs; return *this; };
		CALL Vector3& operator /= ( float rhs ){ y /= rhs; y /= rhs; z /= rhs; return *this; };

		// unary operators
		CALL Vector3 operator + () const { return Vector3(x,y,z); }
		CALL Vector3 operator - () const { return Vector3(-x,-y,-z); }

		// binary operators
		CALL Vector3 operator + ( const Vector3& rhs) const { return Vector3(x+rhs.x, y+rhs.y, z+rhs.z); }
		CALL Vector3 operator - ( const Vector3& rhs) const { return Vector3(x-rhs.x, y-rhs.y, z-rhs.z); };
		CALL Vector3 operator * ( float rhs) const { return Vector3(x*rhs, y*rhs, z*rhs); };
		CALL Vector3 operator / ( float rhs) const { return Vector3(x/rhs, y/rhs, z/rhs); };

		CALL friend Vector3 operator * ( float lhs, const Vector3& rhs) { return Vector3( lhs*rhs.x, lhs*rhs.y, lhs*rhs.z); };

		CALL bool operator == ( const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
		CALL bool operator != ( const Vector3& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; };
    
		float x, y, z;
	};

	class Vector4 {
	public:
	    CALL Vector4() {};
		CALL Vector4( const float * fArr) { CORE_ASSERT(fArr); x = fArr[0]; y = fArr[1]; z = fArr[2]; w = fArr[3]; };
		CALL Vector4( float _x, float _y, float _z, float _w ) { x = _x; y = _y; z = _z; w = _w; };

	    // casting
		CALL operator float* () { return &x; }
		CALL operator const float* () const { return &x; }

		// assignment operators
		CALL Vector4& operator += ( const Vector4& rhs ){ x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
		CALL Vector4& operator -= ( const Vector4& rhs ){ x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; };
		CALL Vector4& operator *= ( float rhs ){ x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; };
		CALL Vector4& operator /= ( float rhs ){ y /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; };

		// unary operators
		CALL Vector4 operator + () const { return Vector4(x,y,z,w); }
		CALL Vector4 operator - () const { return Vector4(-x,-y,-z,-w); }

		// binary operators
		CALL Vector4 operator + ( const Vector4& rhs) const { return Vector4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w); }
		CALL Vector4 operator - ( const Vector4& rhs) const { return Vector4(x-rhs.x, y-rhs.y, z-rhs.z, w+rhs.w); };
		CALL Vector4 operator * ( float rhs) const { return Vector4(x*rhs, y*rhs, z*rhs, w*rhs ); };
		CALL Vector4 operator / ( float rhs) const { return Vector4(x/rhs, y/rhs, z/rhs, w*rhs ); };

		CALL friend Vector4 operator * ( float lhs, const Vector4& rhs) { return Vector4( lhs*rhs.x, lhs*rhs.y, lhs*rhs.z, lhs*rhs.w); };

		CALL bool operator == ( const Vector4& rhs ) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
		CALL bool operator != ( const Vector4& rhs ) const { return x != rhs.x || y != rhs.y || z != rhs.z || w == rhs.w; };
    
		float x, y, z, w;
	};

	class Plane {
	public:
#if PLATFORM != GPU
	    CALL Plane() {};
		CALL Plane( const float * fArr) { CORE_ASSERT(fArr); a = fArr[0]; b = fArr[1]; c = fArr[2]; d = fArr[3]; };
		CALL Plane( float _a, float _b, float _c, float _d ) { a = _a; b = _b; c = _c; d = _d; };
#endif
	    // casting
		CALL operator float* () { return &a; }
		CALL operator const float* () const { return &a; }

		CALL bool operator == ( const Plane& rhs ) const { return a == rhs.a && b == rhs.b && c == rhs.c && d == rhs.d; }
		CALL bool operator != ( const Plane& rhs ) const { return a != rhs.a || b != rhs.b || c != rhs.c || d == rhs.d; };

		float a,b,c,d;
	};
	class Matrix4x4 {
	public:
#if PLATFORM != GPU
		CALL Matrix4x4() {};
		CALL Matrix4x4( const float * fArr ) { CORE_ASSERT(fArr); memcpy(&_11, fArr, sizeof(Matrix4x4)); }
		CALL Matrix4x4( const Matrix4x4& rhs) { memcpy(&_11, (const float*)&rhs, sizeof(Matrix4x4)); }
		CALL Matrix4x4(	float f11, float f12, float f13, float f14,
						float f21, float f22, float f23, float f24,
						float f31, float f32, float f33, float f34,
						float f41, float f42, float f43, float f44 ) {
			_11 = f11; _12 = f12; _13 = f13; _14 = f14;
			_21 = f21; _22 = f22; _23 = f23; _24 = f24;
			_31 = f31; _32 = f32; _33 = f33; _34 = f34;
			_41 = f41; _42 = f42; _43 = f43; _44 = f44;
		}
#endif

		// access grants
		CALL float& operator () ( unsigned int Row, unsigned int Col ) { return m[(Row*4)+Col]; }
		CALL float  operator () ( unsigned int Row, unsigned int Col ) const { return m[(Row*4)+Col]; }

		// casting operators
		CALL operator float* () { return &_11; }
		CALL operator const float* () const { return &_11; }

		// assignment operators
		CALL Matrix4x4& operator *= ( const Matrix4x4& rhs )	{ *this = *this * rhs; return *this; }

		CALL Matrix4x4& operator += ( const Matrix4x4& rhs ) {
			_11 += rhs._11;	_12 += rhs._12;	_13 += rhs._13;	_14 += rhs._14;
			_21 += rhs._21;	_22 += rhs._22;	_23 += rhs._23;	_24 += rhs._24;
			_31 += rhs._31;	_32 += rhs._32;	_33 += rhs._33;	_34 += rhs._34;
			_41 += rhs._41;	_42 += rhs._42;	_43 += rhs._43;	_44 += rhs._44;
			return *this;
		}

		CALL Matrix4x4& operator -= ( const Matrix4x4& rhs ) {
			_11 -= rhs._11;	_12 -= rhs._12;	_13 -= rhs._13;	_14 -= rhs._14;
			_21 -= rhs._21;	_22 -= rhs._22;	_23 -= rhs._23;	_24 -= rhs._24;
			_31 -= rhs._31;	_32 -= rhs._32;	_33 -= rhs._33;	_34 -= rhs._34;
			_41 -= rhs._41;	_42 -= rhs._42;	_43 -= rhs._43;	_44 -= rhs._44;
			return *this;
		}

		CALL Matrix4x4& operator *= ( float rhs ) {
			_11 *= rhs;	_12 *= rhs;	_13 *= rhs;	_14 *= rhs;
			_21 *= rhs;	_22 *= rhs;	_23 *= rhs;	_24 *= rhs;
			_31 *= rhs;	_32 *= rhs;	_33 *= rhs;	_34 *= rhs;
			_41 *= rhs;	_42 *= rhs;	_43 *= rhs;	_44 *= rhs;
			return *this;
		}
		CALL Matrix4x4& operator /= ( float rhs ) {
			float recip = 1.0f / rhs;
			return operator *=( recip );
		}

		// unary operators
		CALL Matrix4x4 operator + () const { return Matrix4x4(*this); }
#if PLATFORM != GPU
		CALL Matrix4x4 operator - () const {
			return Matrix4x4(	-_11, -_12, -_13, -_14,
								-_21, -_22, -_23, -_24,
								-_31, -_32, -_33, -_34,
								-_41, -_42, -_43, -_44 );
		}
#endif


		// binary operators
		CALL Matrix4x4 operator * ( const Matrix4x4& rhs ) const {
			Matrix4x4 ret;
			for(int i=0;i < 4;i++) {
				for(int j=0;j < 4;j++) {
					ret(i,j) = 0;
					for(int k=0;k < 4;k++)
						ret(i,j) += (*this)(i,k) * rhs(k,j);
				}
			}
			return ret;
		}
		CALL Matrix4x4 operator + ( const Matrix4x4& rhs ) const {
			return (Matrix4x4(*this)+=rhs);
		}
		CALL Matrix4x4 operator - ( const Matrix4x4& rhs) const {
			return (Matrix4x4(*this)-=rhs);
		}
		CALL Matrix4x4 operator * ( float rhs ) const {
			return (Matrix4x4(*this)*=rhs);
		}
		CALL Matrix4x4 operator / ( float rhs ) const {
			return (Matrix4x4(*this)/=rhs);
		}

		CALL friend Matrix4x4 operator * ( float lhs, const Matrix4x4& rhs) {
			return (Matrix4x4(rhs) *= lhs );
		}

		bool operator == ( const Matrix4x4& rhs) const {
		    return 0 == memcmp(this, &rhs, sizeof(Matrix4x4));
		}
		bool operator != ( const Matrix4x4& rhs) const {
		    return 0 != memcmp(this, &rhs, sizeof(Matrix4x4));
		}

		union 
		{
			struct 
			{
				float        _11, _12, _13, _14;
				float        _21, _22, _23, _24;
				float        _31, _32, _33, _34;
				float        _41, _42, _43, _44;

			};
			float m[16];
		};

	};

	class Quaternion {
	public:
		CALL Quaternion() {}
		CALL Quaternion( const float* fArr) { CORE_ASSERT(fArr); x = fArr[0]; y = fArr[1]; z = fArr[2]; w = fArr[3]; }
		CALL Quaternion( float _x, float _y, float _z, float _w ) { x = _x; y = _y; z = _z; w = _w; }

		// casting
		CALL operator float* () { return &x; }
		CALL operator const float* () const { return &x; }

		// assignment operators
		CALL Quaternion& operator += ( const Quaternion& rhs ) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
		CALL Quaternion& operator -= ( const Quaternion& rhs ) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
		CALL Quaternion& operator *= ( const Quaternion& rhs ) {	*this = *this * rhs; return *this; }
		CALL Quaternion& operator *= ( float rhs ) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; };
		CALL Quaternion& operator /= ( float rhs ) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; };

		// unary operators
		CALL Quaternion  operator + () const { return Quaternion(x,y,z,w); }
		CALL Quaternion  operator - () const { return Quaternion(-x,-y,-z,-w); }

		// binary operators
		CALL Quaternion operator + ( const Quaternion& rhs ) const { return Quaternion(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w); }
		CALL Quaternion operator - ( const Quaternion& rhs ) const { return Quaternion(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w); }
		CALL Quaternion operator * ( const Quaternion& rhs ) const {
			float qx = rhs.x, qy = rhs.y, qz = rhs.z, qw = rhs.w;
			float rw = w * qw - x * qx - y * qy - z * qz;
			float rx = w * qx + x * qw + y * qz - z * qy;
			float ry = w * qy + y * qw + z * qx - x * qz;
			float rz = w * qz + z * qw + x * qy - y * qx;
			return Quaternion( rx, ry, rz, rw );
		}

		CALL Quaternion operator * ( float rhs ) const { return Quaternion( x*rhs, y*rhs, z*rhs, w*rhs ); }
		CALL Quaternion operator / ( float rhs ) const { return Quaternion( x/rhs, y/rhs, z/rhs, w/rhs ); };

		CALL friend Quaternion operator * (float lhs, const Quaternion& rhs) { return (Quaternion(rhs)*= lhs); }

		CALL bool operator == ( const Quaternion& rhs ) const {
		    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
		}

		CALL bool operator != ( const Quaternion& rhs ) const {
		    return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
		}

		float x, y, z, w;
	};

	CALL inline Vector2 Promote( float x, float y ) { return Vector2(x,y); }
	CALL inline Vector3 Promote( const Vector2& v, float z ) { return Vector3(v.x,v.y,z); }
	CALL inline Vector4 Promote( const Vector3& v, float w ) { return Vector4(v.x,v.y,v.z,w); }

	CALL inline float Dot( const Vector2& vecA, const Vector2& vecB ){ return vecA.x*vecB.x + vecA.y*vecB.y; }   	//!< Dot product of 2 2D Vectors
	CALL inline float Dot( const Vector3& vecA, const Vector3& vecB ){ return vecA.x*vecB.x + vecA.y*vecB.y + vecA.z*vecB.z; }   	//!< Dot product of 2 3D Vectors
	CALL inline float Dot( const Vector4& vecA, const Vector4& vecB ){ return vecA.x*vecB.x + vecA.y*vecB.y + vecA.z*vecB.z + vecA.w*vecB.w; }   	//!< Dot product of 2 4D Vectors
	CALL inline float Dot( const Quaternion& quatA, const Quaternion& quatB ){ return quatA.x*quatB.x + quatA.y*quatB.y + quatA.z*quatB.z + quatA.w*quatB.w; } //! Dot Product of 2 Quaternions
	CALL inline float Dot( const Plane& plane, const Vector4& vec ){ return plane.a*vec.x + plane.b*vec.y + plane.c*vec.z + plane.d*vec.w; }			//!< ax + by + cz + dw
	CALL inline float DotPoint( const Plane& plane, const Vector3& vec ){ return plane.a*vec.x + plane.b*vec.y + plane.c*vec.z + plane.d; }	//!< ax + by + cz + d
	CALL inline float DotNormal( const Plane& plane, const Vector3& vec ){ return plane.a*vec.x + plane.b*vec.y + plane.c*vec.z; }	//!< ax + by + cz

	CALL inline Vector2 ComponentMultiply( const Vector2& vecA, const Vector2& vecB ){ return Math::Vector2( vecA[0]*vecB[0], vecA[1]*vecB[1] ); } 
	CALL inline Vector3 ComponentMultiply( const Vector3& vecA, const Vector3& vecB ){ return Math::Vector3( vecA[0]*vecB[0], vecA[1]*vecB[1], vecA[2]*vecB[2] ); } 
	CALL inline Vector4 ComponentMultiply( const Vector4& vecA, const Vector4& vecB ){ return Math::Vector4( vecA[0]*vecB[0], vecA[1]*vecB[1], vecA[2]*vecB[2], vecA[3]*vecB[3] ); } 

	template <typename T> int Sign(T val) { return (T(0) < val) - (val < T(0)); }

	CALL inline float	Reciprocal( const float& a ) { return 1.0f / a; }
	CALL inline Vector2 Reciprocal( const Vector2& vecA ){ return Math::Vector2( 1.0f / vecA[0], 1.0f / vecA[1] ); } 
	CALL inline Vector3 Reciprocal( const Vector3& vecA ){ return Math::Vector3( 1.0f / vecA[0], 1.0f / vecA[1], 1.0f / vecA[2] ); } 
	CALL inline Vector4 Reciprocal( const Vector4& vecA ){ return Math::Vector4( 1.0f / vecA[0], 1.0f / vecA[1], 1.0f / vecA[2], 1.0f / vecA[3] ); } 

	CALL inline float LengthSquared( const float& a ){ return a * a; }  			//!< Length^2 of a !D Vector
	CALL inline float LengthSquared( const Vector2& vec ){ return Dot(vec,vec); }  	//!< Length^2 of a 2D Vector
	CALL inline float LengthSquared( const Vector3& vec ){ return Dot(vec,vec); }  	//!< Length^2 of a 3D Vector
	CALL inline float LengthSquared( const Vector4& vec ){ return Dot(vec,vec); }  	//!< Length^2 of a 4D Vector
	CALL inline float LengthSquared( const Quaternion& q ){ return Dot(q,q); }  	//!< Length^2 of a 4D Vector

	CALL inline float Length( const float& a ){ return sqrtf(LengthSquared(a)); } 	//!< Length of a 1D Vector
	CALL inline float Length( const Vector2& vec ){ return sqrtf(LengthSquared(vec)); } 	//!< Length of a 2D Vector
	CALL inline float Length( const Vector3& vec ){ return sqrtf(LengthSquared(vec)); } 	//!< Length of a 3D Vector
	CALL inline float Length( const Vector4& vec ){ return sqrtf(LengthSquared(vec)); } 	//!< Length of a 4D Vector
	
	CALL inline float ReciprocalSqrt( const float& a ) { return 1.0f / Length(a); }
	CALL inline float ReciprocalSqrt( const Vector2& vecA ){ return 1.0f / Length(vecA); } 
	CALL inline float ReciprocalSqrt( const Vector3& vecA ){ return 1.0f / Length(vecA); } 
	CALL inline float ReciprocalSqrt( const Vector4& vecA ){ return 1.0f / Length(vecA); } 

	CALL inline float Cross( const Vector2& vecA, const Vector2& vecB ){ return vecA.x*vecB.y - vecA.y*vecB.x; }   //!< Cross product of 2 2D Vectors (CCW this is actually vecA(x,y,0) cross vecB(x,y,0) )
	CALL inline Vector3 Cross( const Vector3& vecA, const Vector3& vecB ) { 
		return Vector3(	vecA.y*vecB.z - vecA.z*vecB.y, vecA.z*vecB.x - vecA.x*vecB.z, vecA.x*vecB.y - vecA.y*vecB.x );
	}   //!< Cross product of 2 3D Vectors 

	CALL inline Vector2 Normalise( const Vector2& vec )	{ return Vector2(vec/Length(vec)); }		//!< returns a normalise version of vec
	CALL inline Vector3 Normalise( const Vector3& vec ){ return Vector3(vec/Length(vec)); }		//!< returns a normalise version of vec
	CALL inline Vector4 Normalise( const Vector4& vec ){ return Vector4(vec/Length(vec)); }		//!< returns a normalise version of vec
	CALL inline Plane Normalise( const Plane& plane ) {
		float len = Length( Vector3(plane.a, plane.b, plane.c) );
		return Plane( plane.a / len, plane.b / len, plane.c / len, plane.d / len ); 
	}

	CALL inline Vector2 Lerp( const Vector2& vecA, const Vector2& vecB, float t){ return vecA + t*(vecB-vecA); } 
	CALL inline Vector3 Lerp( const Vector3& vecA, const Vector3& vecB, float t){ return vecA + t*(vecB-vecA); } 
	CALL inline Vector4 Lerp( const Vector4& vecA, const Vector4& vecB, float t){ return vecA + t*(vecB-vecA); } 
//	inline Quaternion Slerp( const Quaternion& quatA, const Quaternion& quatB, float t ){ Quaternion quat; D3DXQuaternionSlerp( &quat, &quatA, &quatB, t ); return quat; }

	template<typename T>
	CALL inline T Max( const T a, const T b ) { return (a > b)? a : b; }
	template<typename T>
		CALL inline T Min( const T a, const T b ) { return (a < b)? a : b; }

	CALL inline float Clamp( const float a, const float mi, const float ma ) { float r; r = Max(a, mi); r = Min(r, ma); return r; }

	CALL inline Vector2 Abs( const Vector2& vector ) { return Vector2( fabsf( vector.x ), fabsf( vector.y ) ); }
	CALL inline Vector2 Max( const Vector2& vecA, const Vector2& vecB ) { return Vector2( vecA.x > vecB.x ? vecA.x : vecB.x, vecA.y > vecB.y ? vecA.y : vecB.y ); }
	CALL inline Vector2 Min( const Vector2& vecA, const Vector2& vecB ) { return Vector2( vecA.x < vecB.x ? vecA.x : vecB.x, vecA.y < vecB.y ? vecA.y : vecB.y ); }
	CALL inline Vector2 Clamp( const Vector2& vecA, const Vector2& vecB, const Vector2& vecC ) { Vector2 vec; vec = Max(vecA, vecB); vec = Min(vec, vecC); return vec; }

	CALL inline Vector3 Abs( const Vector3& vector ) { return Vector3( fabsf( vector.x ), fabsf( vector.y ), fabsf( vector.z ) ); }
	CALL inline Vector3 Max( const Vector3& vecA, const Vector3& vecB ) { return Vector3( vecA.x > vecB.x ? vecA.x : vecB.x, vecA.y > vecB.y ? vecA.y : vecB.y, vecA.z > vecB.z ? vecA.z : vecB.z ); }
	CALL inline Vector3 Min( const Vector3& vecA, const Vector3& vecB ) { return Vector3( vecA.x < vecB.x ? vecA.x : vecB.x, vecA.y < vecB.y ? vecA.y : vecB.y, vecA.z < vecB.z ? vecA.z : vecB.z ); }
	CALL inline Vector3 Clamp( const Vector3& vecA, const Vector3& vecB, const Vector3& vecC ) { Vector3 vec; vec = Max(vecA, vecB); vec = Min(vec, vecC); return vec; }

	CALL inline Vector4 Abs( const Vector4& vector ) { return Vector4( fabsf( vector.x ), fabsf( vector.y ), fabsf( vector.z ), fabsf( vector.w ) ); }
	CALL inline Vector4 Max( const Vector4& vecA, const Vector4& vecB ) { return Vector4( vecA.x > vecB.x ? vecA.x : vecB.x, vecA.y > vecB.y ? vecA.y : vecB.y, vecA.z > vecB.z ? vecA.z : vecB.z, vecA.w > vecB.w ? vecA.w : vecB.w ); }
	CALL inline Vector4 Min( const Vector4& vecA, const Vector4& vecB ) { return Vector4( vecA.x < vecB.x ? vecA.x : vecB.x, vecA.y < vecB.y ? vecA.y : vecB.y, vecA.z < vecB.z ? vecA.z : vecB.z, vecA.w < vecB.w ? vecA.w : vecB.w ); }
	CALL inline Vector4 Clamp( const Vector4& vecA, const Vector4& vecB, const Vector4& vecC ) { Vector4 vec; vec = Max(vecA, vecB); vec = Min(vec, vecC); return vec; }

	CALL inline Vector4 Transform( const Vector4& vec, const Matrix4x4& matrix ) { 
		return Vector4( matrix(0,0)*vec.x + matrix(1,0)*vec.y + matrix(2,0)*vec.z + matrix(3,0)*vec.w,
						matrix(0,1)*vec.x + matrix(1,1)*vec.y + matrix(2,1)*vec.z + matrix(3,1)*vec.w,
						matrix(0,2)*vec.x + matrix(1,2)*vec.y + matrix(2,2)*vec.z + matrix(3,2)*vec.w,
						matrix(0,3)*vec.x + matrix(1,3)*vec.y + matrix(2,3)*vec.z + matrix(3,3)*vec.w );
	}
	CALL inline Vector4 Transform( const Vector2& vec, const Matrix4x4& matrix ) {  //!< return Vector4(vec,0,1) * matrix
		return Vector4( matrix(0,0)*vec.x + matrix(1,0)*vec.y + matrix(2,0),
						matrix(0,1)*vec.x + matrix(1,1)*vec.y + matrix(3,1),
						matrix(0,2)*vec.x + matrix(1,2)*vec.y + matrix(3,2),
						matrix(0,3)*vec.x + matrix(1,3)*vec.y + matrix(3,3) );
	}
	CALL inline Vector4 Transform( const Vector3& vec, const Matrix4x4& matrix ) {   //!< return Vector4(vec,1) * matrix
		return Vector4( matrix(0,0)*vec.x + matrix(1,0)*vec.y + matrix(2,0)*vec.z + matrix(3,0),
						matrix(0,1)*vec.x + matrix(1,1)*vec.y + matrix(2,1)*vec.z + matrix(3,1),
						matrix(0,2)*vec.x + matrix(1,2)*vec.y + matrix(2,2)*vec.z + matrix(3,2),
						matrix(0,3)*vec.x + matrix(1,3)*vec.y + matrix(2,3)*vec.z + matrix(3,3) );
	}					

	CALL inline Vector2 TransformAndProject( const Vector2& vec, const Matrix4x4& matrix ) { Vector4 out = Transform(vec,matrix); return Vector2(out.x/out.w, out.y/out.w); } //!< return Project(Vector4(vec,0,1) * matrix)
	CALL inline Vector3 TransformAndProject( const Vector3& vec, const Matrix4x4& matrix ){ Vector4 out = Transform(vec,matrix); return Vector3(out.x/out.w, out.y/out.w, out.z/out.w); } //!< return Project(Vector4(vec,1) * matrix)
	CALL inline Vector2 TransformAndDropZ( const Vector2& vec, const Matrix4x4& matrix ) { Vector4 out = Transform(vec,matrix); return Vector2(out.x, out.y); } //!< return (Vector4(vec,0,1) * matrix).xy
	CALL inline Vector3 TransformAndDropW( const Vector3& vec, const Matrix4x4& matrix ) {
		return Vector3( matrix(0,0)*vec.x + matrix(1,0)*vec.y + matrix(2,0)*vec.z + matrix(3,0),
						matrix(0,1)*vec.x + matrix(1,1)*vec.y + matrix(2,1)*vec.z + matrix(3,1),
						matrix(0,2)*vec.x + matrix(1,2)*vec.y + matrix(2,2)*vec.z + matrix(3,2) );
	}
																					
	CALL inline Vector2 TransformNormal( const Vector2& vec, const Matrix4x4& matrix ) { Vector4 out = Transform(Vector4(vec.x,vec.y,0,0), matrix); return Vector2(out.x, out.y); } //!< return Vector4(vec,0,0) * matrix
	CALL inline Vector3 TransformNormal( const Vector3& vec, const Matrix4x4& matrix ) { Vector4 out = Transform(Vector4(vec.x,vec.y,vec.z,0), matrix); return Vector3(out.x, out.y, out.z); } //!< return Vector4(vec,0) * matrix

#if PLATFORM != GPU
	CALL inline Matrix4x4 IdentityMatrix() { return Matrix4x4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1); }
	CALL inline Matrix4x4 TransposeMatrix( const Matrix4x4& mat ) {
		return Matrix4x4(	mat(0,0) , mat(1,0), mat(2,0), mat(3,0),
							mat(0,1) , mat(1,1), mat(2,1), mat(3,1),
							mat(0,2) , mat(1,2), mat(2,2), mat(3,2),
							mat(0,3) , mat(1,3), mat(2,3), mat(3,3) );
	}
#endif
	CALL inline Matrix4x4 MultiplyMatrix( const Matrix4x4& a, const Matrix4x4& b){ return a*b; }

	CALL inline Vector3 GetTranslation( const Matrix4x4& mat ){ return Vector3( mat._41, mat._42, mat._43 ); }
	CALL inline Vector3 GetXAxis( const Matrix4x4& mat ){ return Vector3( mat._11, mat._12, mat._13 ); }
	CALL inline Vector3 GetYAxis( const Matrix4x4& mat ){ return Vector3( mat._21, mat._22, mat._23 ); }
	CALL inline Vector3 GetZAxis( const Matrix4x4& mat ){ return Vector3( mat._31, mat._32, mat._33 ); }
	CALL inline void SetTranslation( Matrix4x4& mat, float x, float y, float z ){ mat._41 = x; mat._42 = y; mat._43 = z; }
	CALL inline void SetTranslation( Matrix4x4& mat, const Vector3& pos){ SetTranslation(mat, pos.x, pos.y, pos.z); }
	CALL inline void SetScale( Matrix4x4& mat, float x, float y, float z ){ mat._11 = x; mat._22 = y; mat._33 = z; }
	CALL inline void SetScale( Matrix4x4& mat, const Vector3& pos){ SetScale(mat, pos.x, pos.y, pos.z); }
	CALL inline void SetXAxis( Matrix4x4& mat, const Vector3& axis ){ mat._11 = axis.x; mat._12 = axis.y; mat._13 = axis.z; }
	CALL inline void SetYAxis( Matrix4x4& mat, const Vector3& axis ){ mat._21 = axis.x; mat._22 = axis.y; mat._23 = axis.z; }
	CALL inline void SetZAxis( Matrix4x4& mat, const Vector3& axis ){ mat._31 = axis.x; mat._32 = axis.y; mat._33 = axis.z; }

	CALL inline Quaternion IdentityQuat() { return Quaternion(0,0,0,1); }

	CALL inline Matrix4x4 CreateXRotationMatrix( const float a ) { 
		Matrix4x4 result; 
        const float c = cos(a);
        const float s = sin(a);

		result(0,0) = 1; result(0,1) = 0;  result(0,2) = 0;  result(0,3) = 0;
	    result(1,0) = 0; result(1,1) = c;  result(1,2) = s; result(1,3) = 0;
	    result(2,0) = 0; result(2,1) = -s; result(2,2) = c;  result(2,3) = 0;
		result(3,0) = 0; result(3,1) = 0;  result(3,2) = 0;  result(3,3) = 1;

		return result; 
	}

	CALL inline Matrix4x4 CreateYRotationMatrix( const float a ) { 
		Matrix4x4 result; 
        const float c = cos(a);
        const float s = sin(a);

		result(0,0) = c;  result(0,1) = 0; result(0,2) = -s; result(0,3) = 0;
	    result(1,0) = 0;  result(1,1) = 1; result(1,2) = 0;	 result(1,3) = 0;
	    result(2,0) = s;  result(2,1) = 0; result(2,2) = c;	 result(2,3) = 0;
	    result(3,0) = 0;  result(3,1) = 0; result(3,2) = 0;	 result(3,3) = 1;

		return result; 
	}

	CALL inline Matrix4x4 CreateZRotationMatrix( const float a ) { 
		Matrix4x4 result; 
        const float c = cos(a);
        const float s = sin(a);

		result(0,0) = c;  result(0,1) = s;	result(0,2) = 0; result(0,3) = 0;
	    result(1,0) = -s; result(1,1) = c;	result(1,2) = 0; result(1,3) = 0;
	    result(2,0) = 0;  result(2,1) = 0;	result(2,2) = 1; result(2,3) = 0;
	    result(3,0) = 0;  result(3,1) = 0;	result(3,2) = 0; result(3,3) = 1;

		return result; 
	}
	CALL inline Matrix4x4 CreateScaleMatrix( const float sx, const float sy, const float sz ){ Matrix4x4 result(IdentityMatrix()); SetScale(result, sx, sy, sz); return result; }
	CALL inline Matrix4x4 CreateTranslationMatrix( const float tx, const float ty, const float tz ){ Matrix4x4 result(IdentityMatrix()); SetTranslation(result, tx, ty, tz); return result; };
	CALL inline Matrix4x4 CreateScaleMatrix( const Vector3& scale ){ return CreateScaleMatrix( scale.x, scale.y, scale.z); }
	CALL inline Matrix4x4 CreateTranslationMatrix( const Vector3& trans ){ return CreateTranslationMatrix(trans.x, trans.y, trans.z); }
	CALL inline Matrix4x4 CreateLookAtMatrix( const Math::Vector3& eye, const Math::Vector3& to, const Math::Vector3& up ){ 
		Matrix4x4 result; 
        const Math::Vector3 f = Normalise(to - eye);
        Math::Vector3 u = Normalise(up);
        const Math::Vector3 s = Normalise(Cross(u, f));
        u = Cross(f, s);

        result(0,0) = s.x;			result(0,1) = u.x;			result(0,2) = f.x;			result(0,3) = 0;
        result(1,0) = s.y;			result(1,1) = u.y;			result(1,2) = f.y;			result(1,3) = 0;
        result(2,0) = s.z;			result(2,1) = u.z;			result(2,2) = f.z;			result(2,3) = 0;
        result(3,0) = -Dot(s, eye); result(3,1) = -Dot(u, eye);	result(3,2) = -Dot(f, eye);	result(3,3) = 1;

		return result; 
	}

//	inline Matrix4x4 CreateYawPitchRollRotationMatrix( const float yawRads, const float pitchRads, const float rollRads ){ Matrix4x4 result; D3DXMatrixRotationYawPitchRoll( &result, yawRads, pitchRads, rollRads ); return result; }
#if PLATFORM != GPU
	CALL inline Matrix4x4 CreateRotationMatrix( const Quaternion& quat ) { 
		const float q1 = quat.x; const float q2 = quat.y; const float q3 = quat.z; const float q0 = quat.w;
		const float q1_2 = q1 * q1; const float q2_2 = q2 * q2; const float q3_2 = q3 * q3; const float q0_2 = q0 * q0;
		return Matrix4x4(	q0_2 + q1_2 - q2_2 - q3_2,	(2*q1*q2)+(2*q0*q3),		(2*q1*q3)-(2*q0*q2),		0,
							(2*q1*q2)-(2*q0*q3),		q0_2 - q1_2 + q2_2 - q3_2,	(2*q2*q3)+(2*q0*q1),		0,
							(2*q1*q3)+(2*q0*q2),		(2*q2*q3)-(2*q0*q1),		q0_2 - q1_2 - q2_2 + q3_2,	0,
							0,							0,							0,							1 );
	}
#endif
	CALL inline Quaternion CreateRotationQuat( const Matrix4x4& m ){ 
		float t, s;
		Quaternion q;

		t = m._11 + m._22 + m._33;
		if (t > 0.0f) {
			s = sqrtf(t + 1.0f);
			q.w = s * 0.5f;
			s = 0.5f / s;

			q.x = (m._32 - m._23) * s;
			q.y = (m._13 - m._31) * s;
			q.z = (m._21 - m._12) * s;
		} else {
			int biggest;
			if (m._11 > m._22) {
				if (m._33 > m._11)
					biggest = 2;//I;	
				else
					biggest = 0;//A;
			} else {
				if (m._33 > m._11)
					biggest = 2;//I;
				else
					biggest = 1;//E;
			}

			switch (biggest) {
				case 0:
					s = sqrtf(m._11 - (m._22 + m._33) + 1.0f);
					if (s > 1e-8f) {

						q.x = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._32 - m._23) * s;
						q.y = (m._12 + m._21) * s;
						q.z = (m._13 + m._31) * s;
						break;
					}
					// I
					s = sqrtf(m._33 - (m._11 + m._22) + 1.0f);
					if (s > 1e-8f) {
						q.z = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._21 - m._12) * s;
						q.x = (m._31 + m._13) * s;
						q.y = (m._32 + m._23) * s;
						break;
					}
					// E
					s = sqrtf(m._22 - (m._33 + m._11) + 1.0f);
					if (s > 1e-8f) {
						q.y = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._13 - m._31) * s;
						q.z = (m._23 + m._32) * s;
						q.x = (m._21 + m._12) * s;
						break;
					}
					break;

				case 1:
					s = sqrtf(m._22 - (m._33 + m._11) + 1.0f);
					if (s > 1e-8f) {
						q.y = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._13 - m._31) * s;
						q.z = (m._23 + m._32) * s;
						q.x = (m._21 + m._12) * s;
						break;
					}
					// I
					s = sqrtf(m._33 - (m._11 + m._22) + 1.0f);
					if (s > 1e-8f) {
						q.z = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._21 - m._12) * s;
						q.x = (m._31 + m._13) * s;
						q.y = (m._32 + m._23) * s;
						break;
					}
					// A
					s = sqrtf(m._11 - (m._22 + m._33) + 1.0f);
					if (s > 1e-8f) {
						q.x = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._32 - m._23) * s;
						q.y = (m._12 + m._21) * s;
						q.z = (m._13 + m._31) * s;
						break;
					}
					break;

				case 2:
					s = sqrtf(m._33 - (m._11 + m._22) + 1.0f);
					if (s > 1e-8f) {
						q.z = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._21 - m._12) * s;
						q.x = (m._31 + m._13) * s;
						q.y = (m._32 + m._23) * s;
						break;
					}
					// A
					s = sqrtf(m._11 - (m._22 + m._33) + 1.0f);
					if (s > 1e-8f) {
						q.x = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._32 - m._23) * s;
						q.y = (m._12 + m._21) * s;
						q.z = (m._13 + m._31) * s;
						break;
					}
					// E
					s = sqrtf(m._22 - (m._33 + m._11) + 1.0f);
					if (s > 1e-8f) {
						q.y = s * 0.5f;
						s = 0.5f / s;
						q.w = (m._13 - m._31) * s;
						q.z = (m._23 + m._32) * s;
						q.x = (m._21 + m._12) * s;
						break;
					}
					break;
				default:
					CORE_ASSERT(false);
			}
		}
		return q;
	}
	CALL inline Quaternion CreateRotationQuat( const Vector3& axis, float angle ) {
		return Quaternion( sinf(angle/2) * axis.x, sinf(angle/2) * axis.y, sinf(angle/2) * axis.z, cosf(angle/2) );
	}

};

#endif // end WEIRD_CORE_VECTOR_MATHS_C_H
