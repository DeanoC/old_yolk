//!-----------------------------------------------------
//!
//! \file vector_maths_pc.h
//! contains the maths namespace stuff for pc
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_VECTOR_MATHS_PC_H
#define WIERD_CORE_VECTOR_MATHS_PC_H


#include <d3d10_1.h>
#include <d3dx10.h>


//! Maths functions live in here
namespace Math
{
	typedef D3DXVECTOR2 Vector2;			//!< 2D float vector
	typedef D3DXVECTOR3 Vector3;			//!< 3D float vector
	typedef D3DXVECTOR4 Vector4;			//!< 4D float vector

	typedef D3DXVECTOR2_16F Vector2F16;		//!< 2D 16 bit float vector
	typedef D3DXVECTOR3_16F Vector3F16;		//!< 2D 16 bit float vector
	typedef D3DXVECTOR4_16F Vector4F16;		//!< 2D 16 bit float vector

	typedef _D3DXMATRIXA16 Matrix4x4;		//!< 4x4 float matrix

	typedef D3DXQUATERNION Quaternion;		//!< Quaternion
	typedef D3DXPLANE Plane;				//!< a 3D Plane

	inline Vector2 Promote( float x, float y ) { return Vector2(x,y); }
	inline Vector3 Promote( const Vector2& v, float z ) { return Vector3(v.x,v.y,z); }
	inline Vector4 Promote( const Vector3& v, float w ) { return Vector4(v.x,v.y,v.z,w); }
	
	inline float Dot( const Vector2& vecA, const Vector2& vecB ){ return D3DXVec2Dot( &vecA, &vecB ); }   	//!< Dot product of 2 2D Vectors
	inline float Dot( const Vector3& vecA, const Vector3& vecB ){ return D3DXVec3Dot( &vecA, &vecB ); }   	//!< Dot product of 2 3D Vectors
	inline float Dot( const Vector4& vecA, const Vector4& vecB ){ return D3DXVec4Dot( &vecA, &vecB ); }   	//!< Dot product of 2 4D Vectors
	inline float Dot( const Quaternion& quatA, const Quaternion& quatB ){ return D3DXQuaternionDot( &quatA, &quatB ); } //! Dot Product of 2 Quaternions
	inline float Dot( const Plane& plane, const Vector4& vec ){ return D3DXPlaneDot( &plane, &vec ); }			//!< ax + by + cz + dw
	inline float DotPoint( const Plane& plane, const Vector3& vec ){ return D3DXPlaneDotCoord( &plane, &vec ); }	//!< ax + by + cz + d
	inline float DotNormal( const Plane& plane, const Vector3& vec ){ return D3DXPlaneDotNormal( &plane, &vec ); }	//!< ax + by + cz

	inline Vector2 ComponentMultiply( const Vector2& vecA, const Vector2& vecB ){ return Math::Vector2( vecA[0]*vecB[0], vecA[1]*vecB[1] ); } 
	inline Vector3 ComponentMultiply( const Vector3& vecA, const Vector3& vecB ){ return Math::Vector3( vecA[0]*vecB[0], vecA[1]*vecB[1], vecA[2]*vecB[2] ); } 
	inline Vector4 ComponentMultiply( const Vector4& vecA, const Vector4& vecB ){ return Math::Vector4( vecA[0]*vecB[0], vecA[1]*vecB[1], vecA[2]*vecB[2], vecA[3]*vecB[3] ); } 

	inline Vector2 Reciprocal( const Vector2& vecA ){ return Math::Vector2( 1.0f / vecA[0], 1.0f / vecA[1] ); } 
	inline Vector3 Reciprocal( const Vector3& vecA ){ return Math::Vector3( 1.0f / vecA[0], 1.0f / vecA[1], 1.0f / vecA[2] ); } 
	inline Vector4 Reciprocal( const Vector4& vecA ){ return Math::Vector4( 1.0f / vecA[0], 1.0f / vecA[1], 1.0f / vecA[2], 1.0f / vecA[3] ); } 

	inline float LengthSquared( const Vector2& vec ){ return D3DXVec2LengthSq( &vec ); }  	//!< Length^2 of a 2D Vector
	inline float LengthSquared( const Vector3& vec ){ return D3DXVec3LengthSq( &vec ); }  	//!< Length^2 of a 3D Vector
	inline float LengthSquared( const Vector4& vec ){ return D3DXVec4LengthSq( &vec ); }  	//!< Length^2 of a 4D Vector

	inline float Length( const Vector2& vec ){ return D3DXVec2Length( &vec ); } 	//!< Length of a 2D Vector
	inline float Length( const Vector3& vec ){ return D3DXVec3Length( &vec ); } 	//!< Length of a 3D Vector
	inline float Length( const Vector4& vec ){ return D3DXVec4Length( &vec ); } 	//!< Length of a 4D Vector

	inline float Cross( const Vector2& vecA, const Vector2& vecB ){ return D3DXVec2CCW( &vecA, &vecB ); }   //!< Cross product of 2 2D Vectors (CCW this is actually vecA(x,y,0) cross vecB(x,y,0) )
	inline Vector3 Cross( const Vector3& vecA, const Vector3& vecB ){ Vector3 out; D3DXVec3Cross( &out, &vecA, &vecB ); return out;}   //!< Cross product of 2 3D Vectors 

	inline Vector2 Normalise( const Vector2& vec )	{ Vector2 norm; D3DXVec2Normalize( &norm, &vec ); return norm; }		//!< returns a normalise version of vec
	inline Vector3 Normalise( const Vector3& vec ){ Vector3 norm; D3DXVec3Normalize( &norm, &vec ); return norm; }			//!< returns a normalise version of vec
	inline Vector4 Normalise( const Vector4& vec ){ Vector4 norm; D3DXVec4Normalize( &norm, &vec ); return norm; }			//!< returns a normalise version of vec
	inline Quaternion Normalise( const Quaternion& quat ){ Quaternion norm; D3DXQuaternionNormalize( &norm, &quat ); return norm; }		//!< returns a normalise version of vec
	inline Plane Normalise( const Plane& plane ){ Plane norm; D3DXPlaneNormalize( &norm, &plane ); return norm; }

	inline Vector2 Lerp( const Vector2& vecA, const Vector2& vecB, float t){ Vector2 vec; D3DXVec2Lerp( &vec, &vecA, &vecB, t ); return vec; } 
	inline Vector3 Lerp( const Vector3& vecA, const Vector3& vecB, float t){ Vector3 vec; D3DXVec3Lerp( &vec, &vecA, &vecB, t ); return vec; } 
	inline Vector4 Lerp( const Vector4& vecA, const Vector4& vecB, float t){ Vector4 vec; D3DXVec4Lerp( &vec, &vecA, &vecB, t ); return vec; } 
	inline Quaternion Slerp( const Quaternion& quatA, const Quaternion& quatB, float t ){ Quaternion quat; D3DXQuaternionSlerp( &quat, &quatA, &quatB, t ); return quat; }

	template<typename T>
	inline T Max( const T a, const T b ) { return (a > b)? a : b; }
	template<typename T>
	inline T Min( const T a, const T b ) { return (a < b)? a : b; }

	inline float Clamp( const float a, const float mi, const float ma ) { float r; r = Max(a, mi); r = Min(r, ma); return r; }

	inline Vector2 Abs( const Vector2& vector ) { return Vector2( fabsf( vector.x ), fabsf( vector.y ) ); }
	inline Vector2 Max( const Vector2& vecA, const Vector2& vecB ) { Vector2 vec; D3DXVec2Maximize( &vec, &vecA, &vecB ); return vec; }
	inline Vector2 Min( const Vector2& vecA, const Vector2& vecB ) { Vector2 vec; D3DXVec2Minimize( &vec, &vecA, &vecB ); return vec; }
	inline Vector2 Clamp( const Vector2& vecA, const Vector2& vecB, const Vector2& vecC ) { Vector2 vec; vec = Max(vecA, vecB); vec = Min(vec, vecC); return vec; }

	inline Vector3 Abs( const Vector3& vector ) { return Vector3( fabsf( vector.x ), fabsf( vector.y ), fabsf( vector.z ) ); }
	inline Vector3 Max( const Vector3& vecA, const Vector3& vecB ) { Vector3 vec; D3DXVec3Maximize( &vec, &vecA, &vecB ); return vec; }
	inline Vector3 Min( const Vector3& vecA, const Vector3& vecB ) { Vector3 vec; D3DXVec3Minimize( &vec, &vecA, &vecB ); return vec; }
	inline Vector3 Clamp( const Vector3& vecA, const Vector3& vecB, const Vector3& vecC ) { Vector3 vec; vec = Max(vecA, vecB); vec = Min(vec, vecC); return vec; }

	inline Vector4 Abs( const Vector4& vector ) { return Vector4( fabsf( vector.x ), fabsf( vector.y ), fabsf( vector.z ), fabsf( vector.w ) ); }
	inline Vector4 Max( const Vector4& vecA, const Vector4& vecB ) { Vector4 vec; D3DXVec4Maximize( &vec, &vecA, &vecB ); return vec; }
	inline Vector4 Min( const Vector4& vecA, const Vector4& vecB ) { Vector4 vec; D3DXVec4Minimize( &vec, &vecA, &vecB ); return vec; }
	inline Vector4 Clamp( const Vector4& vecA, const Vector4& vecB, const Vector4& vecC ) { Vector4 vec; vec = Max(vecA, vecB); vec = Min(vec, vecC); return vec; }

	inline Vector4 Transform( const Vector2& vec, const Matrix4x4& matrix ) { Vector4 res; D3DXVec2Transform(&res, &vec, &matrix); return res; }	//!< return Vector4(vec,0,1) * matrix
	inline Vector4 Transform( const Vector3& vec, const Matrix4x4& matrix ) { Vector4 res; D3DXVec3Transform(&res, &vec, &matrix); return res; }	//!< return Vector4(vec,1) * matrix
	inline Vector4 Transform( const Vector4& vec, const Matrix4x4& matrix ) { Vector4 res; D3DXVec4Transform(&res, &vec, &matrix); return res; }	//!< return vec * matrix
	inline Plane Transform( const Plane& plane, const Matrix4x4& matrix ) { Plane res; D3DXPlaneTransform(&res, &plane, &matrix); return res; }		//!< return plane * matrix, plane.abc must be normalised, matrix should be inverse transpose

	inline Vector2 TransformAndProject( const Vector2& vec, const Matrix4x4& matrix ) { Vector2 res; D3DXVec2TransformCoord(&res, &vec, &matrix); return res; } //!< return Project(Vector4(vec,0,0) * matrix)
	inline Vector3 TransformAndProject( const Vector3& vec, const Matrix4x4& matrix ) { Vector3 res; D3DXVec3TransformCoord(&res, &vec, &matrix); return res; } //!< return Project(Vector4(vec,0) * matrix)
	inline Vector2 TransformNormal( const Vector2& vec, const Matrix4x4& matrix ) { Vector2 res; D3DXVec2TransformNormal(&res, &vec, &matrix); return res; } //!< return Vector4(vec,0,0) * matrix
	inline Vector3 TransformNormal( const Vector3& vec, const Matrix4x4& matrix ) { Vector3 res; D3DXVec3TransformNormal(&res, &vec, &matrix); return res; } //!< return Vector4(vec,0) * matrix

	inline Matrix4x4 IdentityMatrix() { Matrix4x4 matrix; D3DXMatrixIdentity( &matrix ); return matrix; }
	inline Matrix4x4 InverseMatrix( const Matrix4x4& mat ){ Matrix4x4 result; D3DXMatrixInverse( &result, 0, &mat ); return result; }
	inline Matrix4x4 TransposeMatrix( const Matrix4x4& mat ){ Matrix4x4 result; D3DXMatrixTranspose( &result, &mat ); return result; }
	inline Matrix4x4 MultiplyMatrix( const Matrix4x4& a, const Matrix4x4& b){ Matrix4x4 result; D3DXMatrixMultiply( &result, &a, &b ); return result; }


	inline Matrix4x4 CreateXRotationMatrix( const float angleRads ){ Matrix4x4 result; D3DXMatrixRotationX( &result, angleRads ); return result; }
	inline Matrix4x4 CreateYRotationMatrix( const float angleRads ){ Matrix4x4 result; D3DXMatrixRotationY( &result, angleRads ); return result; }
	inline Matrix4x4 CreateZRotationMatrix( const float angleRads ){ Matrix4x4 result; D3DXMatrixRotationZ( &result, angleRads ); return result; }
	inline Matrix4x4 CreateYawPitchRollRotationMatrix( const float yawRads, const float pitchRads, const float rollRads ){ Matrix4x4 result; D3DXMatrixRotationYawPitchRoll( &result, yawRads, pitchRads, rollRads ); return result; }
	inline Matrix4x4 CreateRotationMatrix( const Quaternion& quat ){ Matrix4x4 result; D3DXMatrixRotationQuaternion( &result, &quat ); return result; }
	inline Matrix4x4 CreateScaleMatrix( const float sx, const float sy, const float sz ){ Matrix4x4 result; D3DXMatrixScaling( &result, sx, sy, sz ); return result; }
	inline Matrix4x4 CreateTranslationMatrix( const float tx, const float ty, const float tz ){ Matrix4x4 result; D3DXMatrixTranslation( &result, tx, ty, tz ); return result; }
	inline Matrix4x4 CreateScaleMatrix( const Vector3& scale ){ return CreateScaleMatrix( scale.x, scale.y, scale.z); }
	inline Matrix4x4 CreateTranslationMatrix( const Vector3& trans ){ return CreateTranslationMatrix(trans.x, trans.y, trans.z); }
	inline Matrix4x4 CreateLookAtMatrix( const Math::Vector3& eye, const Math::Vector3& to, const Math::Vector3& up ){ Matrix4x4 result; D3DXMatrixLookAtLH( &result, &eye, &to, &up ); return result; }

	inline Vector3 GetTranslation( const Matrix4x4& mat ){ return Vector3( mat._41, mat._42, mat._43 ); }
	inline Vector3 GetXAxis( const Matrix4x4& mat ){ return Vector3( mat._11, mat._12, mat._13 ); }
	inline Vector3 GetYAxis( const Matrix4x4& mat ){ return Vector3( mat._21, mat._22, mat._23 ); }
	inline Vector3 GetZAxis( const Matrix4x4& mat ){ return Vector3( mat._31, mat._32, mat._33 ); }
	inline void SetTranslation( Matrix4x4& mat, float x, float y, float z ){ mat._41 = x; mat._42 = y; mat._43 = z; }
	inline void SetTranslation( Matrix4x4& mat, const Vector3& pos){ SetTranslation(mat, pos.x, pos.y, pos.z); }
	inline void SetScale( Matrix4x4& mat, float x, float y, float z ){ mat._11 = x; mat._22 = y; mat._33 = z; }
	inline void SetScale( Matrix4x4& mat, const Vector3& pos){ SetScale(mat, pos.x, pos.y, pos.z); }
	inline void SetXAxis( Matrix4x4& mat, const Vector3& axis ){ mat._11 = axis.x; mat._12 = axis.y; mat._13 = axis.z; }
	inline void SetYAxis( Matrix4x4& mat, const Vector3& axis ){ mat._21 = axis.x; mat._22 = axis.y; mat._23 = axis.z; }
	inline void SetZAxis( Matrix4x4& mat, const Vector3& axis ){ mat._31 = axis.x; mat._32 = axis.y; mat._33 = axis.z; }

	inline Quaternion IdentityQuat() { Quaternion quat; D3DXQuaternionIdentity( &quat ); return quat; }
	inline Quaternion CreateRotationQuat( const Matrix4x4& mat ){ Quaternion quat; D3DXQuaternionRotationMatrix(&quat,&mat); return quat; }
	inline Quaternion CreateRotationQuat( const Vector3& axis, float angle ){ Quaternion quat; D3DXQuaternionRotationAxis(&quat,&axis,angle); return quat; }

	inline Plane CreatePlane3Points( const Vector3& a, const Vector3& b, const Vector3& c ){ Plane pl; D3DXPlaneFromPoints(&pl,&a,&b,&c); return pl; }
	inline Plane CreatePlanePointNormal( const Vector3& pnt, const Vector3& normal ){ Plane pl; D3DXPlaneFromPointNormal(&pl,&pnt,&normal); return pl; }
};

#define PLATFORM_SPECIFIC_VECTOR_MATH_DEFINED

#endif