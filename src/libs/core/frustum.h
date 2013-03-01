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
	Math::Plane planes[ 6 ];
	Math::Matrix4x4 matrix;

public:
#if COMPILER != CUDA_COMPILER
	CALL Frustum() {};

	Frustum( const Math::Matrix4x4& _matrix ) {
		createFromMatrix( _matrix );
	}
#endif

	//! distance from the frustum to a point
	//! if the point is inside, the return value will be < 0
	CALL float distanceToPoint( const Math::Vector3& point ) const {
		float closestPoint = FLT_MAX;
		for( int i=0; i < 6; ++i ) {
			closestPoint = Math::Min( Math::DotPoint( planes[i], point ), closestPoint );
		}
		return closestPoint;
	}

	//!Taking an AABB min and max in world space, work out its interaction with the view frustum
	CALL CULL_RESULT cullAABB( const Core::AABB& oAABB ) const {
		bool intersect = false;
		Math::Vector3 vMin, vMax;

		for (int i=0; i<6; i++) {
			vMin = oAABB.getMinExtent();
			vMax = oAABB.getMaxExtent();

			if (planes[i].a > 0) {
				vMin.x = oAABB.getMaxExtent().x;
				vMax.x = oAABB.getMinExtent().x;
			}
			if (planes[i].b > 0) {
				vMin.y = oAABB.getMaxExtent().y;
				vMax.y = oAABB.getMinExtent().y;
			}
			if (planes[i].c > 0) {
				vMin.z = oAABB.getMaxExtent().z;
				vMax.z = oAABB.getMinExtent().z;
			}

			if(  Math::DotPoint(planes[i], vMin) < 0) {
				return OUTSIDE;
			}
			if( Math::DotPoint(planes[i], vMax) <= 0) {
				intersect = true;
			}
		}

		return (intersect) ? CROSSING : INSIDE;
	}

	CALL void createFromMatrix( const Math::Matrix4x4& _matrix ) {
		matrix = _matrix;

		// Left clipping plane
		planes[0].a = matrix._14 + matrix._11;
		planes[0].b = matrix._24 + matrix._21;
		planes[0].c = matrix._34 + matrix._31;
		planes[0].d = matrix._44 + matrix._41;

		// Right clipping plane
		planes[1].a = matrix._14 - matrix._11;
		planes[1].b = matrix._24 - matrix._21;
		planes[1].c = matrix._34 - matrix._31;
		planes[1].d = matrix._44 - matrix._41;

		// Top clipping plane
		planes[2].a = matrix._14 - matrix._12;
		planes[2].b = matrix._24 - matrix._22;
		planes[2].c = matrix._34 - matrix._32;
		planes[2].d = matrix._44 - matrix._42;

		// Bottom clipping plane
		planes[3].a = matrix._14 + matrix._12;
		planes[3].b = matrix._24 + matrix._22;
		planes[3].c = matrix._34 + matrix._32;
		planes[3].d = matrix._44 + matrix._42;

		// Near clipping plane
		planes[4].a = matrix._13;
		planes[4].b = matrix._23;
		planes[4].c = matrix._33;
		planes[4].d = matrix._43;

		// Far clipping plane
		planes[5].a = matrix._14 - matrix._13;
		planes[5].b = matrix._24 - matrix._23;
		planes[5].c = matrix._34 - matrix._33;
		planes[5].d = matrix._44 - matrix._43;

		for ( int i = 0; i < 6; ++i ) {
			planes[i] = Math::Normalise( planes[i] );
		}
	}
	void getWorldSpacePoints( Math::Vector3 pts[8] ) const {
		//     e----h
		//	  /|   /|
		//	 / f--/-g
		/// /  / /  /
		//	a---d  /   
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

		Math::Matrix4x4 inv = Math::InverseMatrix( matrix );

		pts[0] = Math::TransformAndProject( a, inv );
		pts[1] = Math::TransformAndProject( b, inv );
		pts[2] = Math::TransformAndProject( c, inv );
		pts[3] = Math::TransformAndProject( d, inv );
		pts[4] = Math::TransformAndProject( e, inv );
		pts[5] = Math::TransformAndProject( f, inv );
		pts[6] = Math::TransformAndProject( g, inv );
		pts[7] = Math::TransformAndProject( h, inv );

	}

#if PLATFORM != GPU
	void debugDraw( const Core::Colour& colour ) const {
		Math::Vector3 pts[8];
		getWorldSpacePoints( pts );

		g_pDebugRender->worldRect( Core::RGBAColour(1,1,1,1), pts[0], pts[1], pts[2], pts[3] );
		g_pDebugRender->worldRect( colour, pts[4], pts[5], pts[6], pts[7] );
		g_pDebugRender->worldRect( colour, pts[0], pts[1], pts[5], pts[4] );
		g_pDebugRender->worldRect( colour, pts[2], pts[3], pts[7], pts[6] );
	}
#endif
};


}	//namespace Core


#endif
