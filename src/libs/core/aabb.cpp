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
	/// \fn	AABB AABB::transform( const Math::Matrix4x4& matrix ) const
	///
	/// \brief	Transforms this AABB by the parameter. 
	///
	/// \param	matrix	The transform matrix. 
	///
	/// \return	Transformed AABB. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	AABB AABB::transform( const Math::Matrix4x4& matrix ) const
	{
		// start search at the transform point
		AABB ret( Math::GetTranslation( matrix ), Math::GetTranslation( matrix ) );

		// Find extreme points by considering product of 
		// min and max with each component of trans.
		for ( int j = 0; j < 3; ++j )
		{
			for ( int i = 0; i < 3; ++i )
			{
				const float a = matrix( i, j ) * m_MinExtent[ i ];
				const float b = matrix( i, j ) * m_MaxExtent[ i ];

				if ( a < b )
				{
					ret.m_MinExtent[ j ] += a;
					ret.m_MaxExtent[ j ] += b;
				}
				else
				{
					ret.m_MinExtent[ j ] += b;
					ret.m_MaxExtent[ j ] += a;
				}
			}
		}

		return ret;
	}


}
