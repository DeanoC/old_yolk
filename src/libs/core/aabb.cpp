///-------------------------------------------------------------------------------------------------
/// \file	core\aabb.cpp
///
/// \brief	Implements the aabb class. 
///
/// \details	
///		aabb description goes here
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "core/core.h"
#include "aabb.h"

///-------------------------------------------------------------------------------------------------
/// \namespace	Core
///
/// \brief	.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Core
{
	///-------------------------------------------------------------------------------------------------
	/// \fn	AABB AABB::transformAffine( const Math::Matrix4x4& matrix ) const
	///
	/// \brief	Transforms this AABB by the affine matrix parameter. 
	///
	/// \param	matrix	An affine transform matrix. 
	///
	/// \return	Transformed AABB. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	AABB AABB::transformAffine( const Math::Matrix4x4& matrix ) const {
		// start search at the transform point
		AABB ret( Math::GetTranslation( matrix ), Math::GetTranslation( matrix ) );

		// Find extreme points by considering product of 
		// min and max with each component of trans.
		for ( int j = 0; j < 3; ++j ) {
			for ( int i = 0; i < 3; ++i ) {
				const float a = matrix( i, j ) * m_MinExtent[ i ];
				const float b = matrix( i, j ) * m_MaxExtent[ i ];

				if ( a < b ) {
					ret.m_MinExtent[ j ] += a;
					ret.m_MaxExtent[ j ] += b;
				} else {
					ret.m_MinExtent[ j ] += b;
					ret.m_MaxExtent[ j ] += a;
				}
			}
		}

		return ret;
	}

	///-------------------------------------------------------------------------------------------------
	/// \fn	AABB AABB::transform( const Math::Matrix4x4& matrix ) const
	///
	/// \brief	Transforms this AABB by the parameter. 
	///
	/// \param	matrix	The transform matrix. 
	///
	/// \return	Transformed AABB. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	AABB AABB::transform( const Math::Matrix4x4& matrix ) const {

		AABB ret;

		for( int i = 0;i < 8; ++i ) {
			const bool maxX = !!(i & 0x1);
			const bool maxY = !!(i & 0x2);
			const bool maxZ = !!(i & 0x4);
			Math::Vector3 pt;
			pt[0] = maxX ? m_MaxExtent[ 0 ] : m_MinExtent[ 0 ];
			pt[1] = maxY ? m_MaxExtent[ 1 ] : m_MinExtent[ 1 ];
			pt[2] = maxZ ? m_MaxExtent[ 2 ] : m_MinExtent[ 2 ];

			pt = Math::TransformAndProject( pt, matrix );
			ret.expandBy( pt );
		}

		return ret;
	}


}
