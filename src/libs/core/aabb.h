/// \file	aabb.h
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
/// \brief	A class representing a axis aligned bounding box

#pragma once
#ifndef YOLK_CORE_AABB_H
#define YOLK_CORE_AABB_H

#include "vector_math.h"
#include "mathconsts.h"
#if PLATFORM != GPU
#	include "debug_render.h"
#endif

namespace Core {

/// \brief	Aabb. 
class AABB {
public:
	/// Default constructor. AABB is invalid.
	CALL explicit AABB();

	/// Constructor.
	/// \param	min	The minimum extents as a vector.
	/// \param	max	The maximum extents as a vector.
	CALL explicit AABB( const Math::Vector3& min, const Math::Vector3& max );

	/// Constructor.
	/// \param	min	The minimum extents as a float array.
	/// \param	max	The maximum extents as a float array.
	CALL explicit AABB( const float min[3], const float max[3] );

#if PLATFORM != GPU
	static AABB fromCenterAndHalfLength( const Math::Vector3& center, const Math::Vector3& halfLength );
#endif

	/// Query if this object is valid.
	/// \return	true if valid, false if not.
	CALL bool isValid() const;

	/// Gets the minimum extent.
	/// \return	The minimum extent.
	CALL const Math::Vector3& getMinExtent() const;

	/// Gets the maximum extent.
	/// \return	The maximum extent.
	CALL const Math::Vector3& getMaxExtent() const;

	/// Gets the box center.
	/// \return	The box center.
	CALL Math::Vector3 getBoxCenter() const;

	/// Gets the half length.
	/// \return	The half length.
	CALL Math::Vector3 getHalfLength() const;

	/// Sets a box center.
	/// \param	center	The center.
	CALL void setBoxCenter( const Math::Vector3& center );

	/// expand (AKA union) by a pnt. 
	/// \param	pnt	The pnt.
	CALL void expandBy( const Math::Vector3& pnt );

	/// expand (AKA union) by another aabb.
	/// \param	aabb	The aabb.
	CALL void expandBy( const AABB& aabb );

	/// Union this box with the parameter.
	/// \param	aabb	The aabb.
	CALL void unionWith( const AABB& aabb );

	/// This box becomes the intersection of the param and itself.
	/// \param	aabb	The aabb.
	CALL void intersectWith( const AABB& aabb );

	/// Query if this object intersects the given aabb.
	/// \param	aabb	The aabb.
	/// \return	true if it succeeds, false if it fails.
	CALL bool intersects( const AABB& aabb ) const;

	/// Query if this object intersects the point.
	/// \param	_point	The point.
	/// \return	true if it succeeds, false if it fails.
	CALL bool intersects( const Math::Vector3& _point ) const;

	/// Transforms this AABB by the param.
	/// \param	matrix	The matrix.
	/// \return	The transformed AABB.
	CALL AABB transform( const Math::Matrix4x4& matrix ) const;

#if PLATFORM != GPU
	/// Draw debug the AABB.
	/// \param	colour	The colour.
	/// \param	trans 	(optional) the trans.
	void drawDebug( const Core::Colour& colour, const Math::Matrix4x4& trans = Math::IdentityMatrix() ) const;
#endif

private:
	Math::Vector3 m_MinExtent;	//!< Minimum extent of the box
	Math::Vector3 m_MaxExtent;	//!< Maximum extent of the box
};

#	include "aabb.inl"

}	//namespace Core


#endif
