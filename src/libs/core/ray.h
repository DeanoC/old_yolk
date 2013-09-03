///-------------------------------------------------------------------------------------------------
/// \file	core\ray.h
///
/// \brief	Declares the ray class.
///
/// \details	
///		ray description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef WIERD_CORE_RAY_H
#define WIERD_CORE_RAY_H

#include "vector_math.h"
#include "mathconsts.h"
#include "aabb.h"

namespace Core {
	///-------------------------------------------------------------------------------------------------
	/// \class	Ray
	///
	/// \brief	Aabb.
	///
	/// \todo	A class representing a axis aligned bounding box.
	/// based on meshtool ray (see copyright message in other source files)
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class Ray {
	public:
		Ray();
		Ray( Math::Vector3 const& origin, Math::Vector3 const& direction );

		Math::Vector3 const& getOrigin() const { return m_origin; }
		Math::Vector3 const& getDirection() const { return m_direction; }

		//! Ray-AABB intersection test.
		bool intersectsAABB( AABB const& bounds, float& min, float& max ) const;

		//! Ray-triangle intersection test.
		bool intersectsTriangle( 
			Math::Vector3 const& v0, Math::Vector3 const& v1, Math::Vector3 const& v2, 
			float& v, float& w, float& t 
			) const;

	private:
		Math::Vector3 m_origin;
		Math::Vector3 m_direction;
	};
}

#endif