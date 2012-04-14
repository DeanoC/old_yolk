//!-----------------------------------------------------
//!
//! \file frustum.h
//! A class representing a frustum
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_FRUSTUM_H
#define WIERD_CORE_FRUSTUM_H


#include "vector_math.h"
#include "aabb.h"


namespace Core {


class Frustum {
public:
	enum CULL_RESULT {
		OUTSIDE = 0,
		CROSSING = 1,
		INSIDE = 2,
	};

public:
	Math::Plane m_Planes[ 6 ];
	Math::Matrix4x4 m_Matrix;

public:
#if COMPILER != CUDA_COMPILER
	CALL Frustum() {};

	Frustum( const Math::Matrix4x4& Matrix ) {
		createFromMatrix( Matrix );
	}
#endif

	//! distance from the frustum to a point
	//! if the point is inside, the return value will be < 0
	CALL float distanceToPoint( const Math::Vector3& Point ) const {
		float closestPoint = FLT_MAX;
		for( int i=0; i < 6; ++i ) {
			closestPoint = Math::Min( Math::DotPoint( m_Planes[i], Point ), closestPoint );
		}
		return closestPoint;
	}

	//!Taking an AABB min and max in world space, work out its interaction with the view frustum
	CALL CULL_RESULT cullAABB( const Core::AABB& oAABB ) {
		bool Intersect = false;
		Math::Vector3 Min, Max;

		for (int i=0; i<6; i++) {
			Min = oAABB.getMinExtent();
			Max = oAABB.getMaxExtent();

			if (m_Planes[i].a > 0) {
				Min.x = oAABB.getMaxExtent().x;
				Max.x = oAABB.getMinExtent().x;
			}
			if (m_Planes[i].b > 0) {
				Min.y = oAABB.getMaxExtent().y;
				Max.y = oAABB.getMinExtent().y;
			}
			if (m_Planes[i].c > 0) {
				Min.z = oAABB.getMaxExtent().z;
				Max.z = oAABB.getMinExtent().z;
			}

			if(  Math::DotPoint(m_Planes[i], Min) < 0) {
				return OUTSIDE;
			}
			if( Math::DotPoint(m_Planes[i], Max) <= 0) {
				Intersect = true;
			}
		}

		return (Intersect) ? CROSSING : INSIDE;
	}

	CALL void createFromMatrix( const Math::Matrix4x4& Matrix ) {
		m_Matrix = Matrix;

		// Left clipping plane
		m_Planes[0].a = Matrix._14 + Matrix._11;
		m_Planes[0].b = Matrix._24 + Matrix._21;
		m_Planes[0].c = Matrix._34 + Matrix._31;
		m_Planes[0].d = Matrix._44 + Matrix._41;

		// Right clipping plane
		m_Planes[1].a = Matrix._14 - Matrix._11;
		m_Planes[1].b = Matrix._24 - Matrix._21;
		m_Planes[1].c = Matrix._34 - Matrix._31;
		m_Planes[1].d = Matrix._44 - Matrix._41;

		// Top clipping plane
		m_Planes[2].a = Matrix._14 - Matrix._12;
		m_Planes[2].b = Matrix._24 - Matrix._22;
		m_Planes[2].c = Matrix._34 - Matrix._32;
		m_Planes[2].d = Matrix._44 - Matrix._42;

		// Bottom clipping plane
		m_Planes[3].a = Matrix._14 + Matrix._12;
		m_Planes[3].b = Matrix._24 + Matrix._22;
		m_Planes[3].c = Matrix._34 + Matrix._32;
		m_Planes[3].d = Matrix._44 + Matrix._42;

		// Near clipping plane
		m_Planes[4].a = Matrix._13;
		m_Planes[4].b = Matrix._23;
		m_Planes[4].c = Matrix._33;
		m_Planes[4].d = Matrix._43;

		// Far clipping plane
		m_Planes[5].a = Matrix._14 - Matrix._13;
		m_Planes[5].b = Matrix._24 - Matrix._23;
		m_Planes[5].c = Matrix._34 - Matrix._33;
		m_Planes[5].d = Matrix._44 - Matrix._43;

		for ( int i = 0; i < 6; ++i ) {
			m_Planes[i] = Math::Normalise( m_Planes[i] );
		}
	}

#if PLATFORM != GPU
	void debugDraw( const Core::Colour& colour ) const {
		//     e----h
		//	  /|   /|
		//	 / f--|-g
		/// /  / /  /
		//	a---d /   
		//	|/   |/
		//  b___c

		Math::Vector3 a( -1, -1, 0 );
		Math::Vector3 b( -1,  1, 0 );
		Math::Vector3 c(  1,  1, 0 );
		Math::Vector3 d(  1, -1, 0 );
		Math::Vector3 e( -1, -1, 0.99999f );
		Math::Vector3 f( -1,  1, 0.99999f );
		Math::Vector3 g(  1,  1, 0.99999f );
		Math::Vector3 h(  1, -1, 0.99999f );

		Math::Matrix4x4 inv = Math::InverseMatrix( m_Matrix );

		a = Math::TransformAndProject( a, inv );
		b = Math::TransformAndProject( b, inv );
		c = Math::TransformAndProject( c, inv );
		d = Math::TransformAndProject( d, inv );
		e = Math::TransformAndProject( e, inv );
		f = Math::TransformAndProject( f, inv );
		g = Math::TransformAndProject( g, inv );
		h = Math::TransformAndProject( h, inv );

		g_pDebugRender->worldRect( colour, a, b, c, d );
		g_pDebugRender->worldRect( colour, e, f, g, h );
		g_pDebugRender->worldRect( colour, a, b, f, e );
		g_pDebugRender->worldRect( colour, c, d, h, g );
	}
#endif
};


}	//namespace Core


#endif
