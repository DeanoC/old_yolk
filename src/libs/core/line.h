//!-----------------------------------------------------
//!
//! \file line.h
//! contains the simple 3D line and line segment class
//!
//!-----------------------------------------------------


#pragma once

#ifndef WIERD_CORE_LINE_H
#define WIERD_CORE_LINE_H


namespace Math {


//! A Line Segment is a line that has a distinct begining and end and
//! is only valid inside that segment, i.e. it starts at p0 and ends
//! at p1 and only points between them are valid
class LineSegment3 {
public:
	LineSegment3(){};
	LineSegment3( const Vector3& p0, const Vector3& p1) :
		m_p0(p0), m_p1(p1) {}

	Vector3 NearestPointOnLine( const Vector3& point ) const {
		Vector3 b_minus_a = m_p1 - m_p0;
		Vector3 p_minus_a = point - m_p0;

		// check if off m_p0 vertex
		float dot_ta = Dot(p_minus_a,b_minus_a);
		if( dot_ta <= 0.f)
			return m_p0;

		float dot_bma = Dot(b_minus_a,b_minus_a);
		if (dot_bma <= dot_ta)
			return m_p1;

		float b = dot_ta / dot_bma;
		return (m_p0 + b * b_minus_a);
	}

	float MinDistanceToPoint( const Vector3& point ) const {
		Vector3 closestPt = NearestPointOnLine(point);
		return Math::Length( point - closestPt );
	}

	float SqrDistance( const Vector3& point ) const {
		Vector3 kDiff = point - m_p0;
		Vector3 dir = m_p1 - m_p0;
		float rT = Dot(kDiff, dir);

		if( rT > 0 ) {
			float rSqrLen = Dot(dir, dir);
			if( rT >= rSqrLen ) {
				kDiff -= dir;
			} else {
				rT /= rSqrLen;
				kDiff -= rT * dir;
			}
		}
		return Dot(kDiff,kDiff);
	}

	float Distance( const Vector3& point ) const {
		return sqrtf( (float)SqrDistance(point) );
	}

protected:
	Vector3	m_p0, m_p1;

};


//! A line is an infinite line that passes though the two exampler
//! points passed in. Any point along the infinite line is valid
class Line3 {
public:
	Line3() {}
	Line3( const Vector3& p0, const Vector3& p1) :
		m_p0(p0), m_p1(p1) {}

	Vector3 NearestPointOnLine( const Vector3& point ) const {
		Vector3 b_minus_a = m_p1 - m_p0;
		Vector3 p_minus_a = point - m_p0;

		// check if off m_p0 vertex
		float dot_ta = Dot( p_minus_a, b_minus_a);
		float dot_bma = Dot( b_minus_a, b_minus_a);

		float b = dot_ta / dot_bma;
		return (m_p0 + b * b_minus_a);
	}

	float MinDistanceToPoint( const Vector3& point ) const {
		Vector3 closestPt = NearestPointOnLine(point);
		return Math::Length( point - closestPt );
	}

protected:
	Vector3	m_p0, m_p1;

};


}	//namespace Math


#endif
