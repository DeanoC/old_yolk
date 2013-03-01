///-------------------------------------------------------------------------------------------------
/// \file	core\aabb.inl
///
/// \brief	aabb class.
///
/// \details	
///		aabb description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////

///-------------------------------------------------------------------------------------------------
/// \brief	Default constructor, AABB is invalid. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE AABB::AABB() : 
m_MinExtent( Math::infinity<float>(), Math::infinity<float>(), Math::infinity<float>() ),
m_MaxExtent( -Math::infinity<float>(), -Math::infinity<float>(), -Math::infinity<float>() )
{
}

///-------------------------------------------------------------------------------------------------
/// \brief	Constructor, set via the minimum and maximum extents. 
///
/// \param	min	The minimum extent of the box. 
/// \param	max	The maximum extent of the box. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE AABB::AABB( const Math::Vector3& min, const Math::Vector3& max ) :
m_MinExtent( min ),
m_MaxExtent( max )
{
}

///-------------------------------------------------------------------------------------------------
/// \brief	Constructor, set via the minimum and maximum extents. 
///
/// \param	min	The minimum extent of the box. 
/// \param	max	The maximum extent of the box. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE AABB::AABB( const float min[3], const float max[3] ) :
m_MinExtent( min[0], min[1], min[2] ),
m_MaxExtent( max[0], max[1], max[2] )
{
}

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE bool AABB::isValid() const
///
/// \brief	Query if this object is valid. 
///
/// \return	true if valid, false if not. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE bool AABB::isValid() const
{
	return Math::LengthSquared(m_MaxExtent - m_MinExtent) > 0.0f;
}


///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE const Math::Vector3& AABB::getMinExtent() const
///
/// \brief	Gets the minimum extent. 
///
/// \return	The minimum extent. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE const Math::Vector3& AABB::getMinExtent() const
{
	return m_MinExtent;
}

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE const Math::Vector3& AABB::getMaxExtent() const
///
/// \brief	Gets the maximum extent. 
///
/// \return	The maximum extent. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE const Math::Vector3& AABB::getMaxExtent() const
{
	return m_MaxExtent;
}


///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE Math::Vector3 AABB::getBoxCenter() const
///
/// \brief	Gets the box center. 
///
/// \return	The box center. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE Math::Vector3 AABB::getBoxCenter() const
{
	return (m_MaxExtent + m_MinExtent) * 0.5f;
}

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE Math::Vector3 AABB::getHalfLength() const
///
/// \brief	Gets the half length. 
///
/// \return	The half length. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE Math::Vector3 AABB::getHalfLength() const
{
	return (m_MaxExtent - m_MinExtent) * 0.5f;
}


///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE void AABB::setBoxCenter( const Math::Vector3& center )
///
/// \brief	Sets a box center. 
///
/// \param	center	The center. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE void AABB::setBoxCenter( const Math::Vector3& center )
{
	const Math::Vector3 oldCenter = getBoxCenter();
	m_MinExtent = (m_MinExtent - oldCenter) + center;
	m_MaxExtent = (m_MaxExtent - oldCenter) + center;
}

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE void AABB::expandBy( const Math::Vector3& pnt )
///
/// \brief	Expand (AKA union) by a pnt. 
///
/// \param	pnt	The pnt. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE void AABB::expandBy( const Math::Vector3& pnt )
{
	m_MinExtent = Math::Min( m_MinExtent, pnt );
	m_MaxExtent = Math::Max( m_MaxExtent, pnt );
}

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE void AABB::expandBy( const AABB& aabb )
///
/// \brief	Expand (AKA union) this box by another aabb. 
///
/// \param	aabb	The aabb. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE void AABB::expandBy( const AABB& aabb )
{
	m_MinExtent = Math::Min( m_MinExtent, aabb.m_MinExtent );
	m_MaxExtent = Math::Max( m_MaxExtent, aabb.m_MaxExtent );
}

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE void AABB::unionWith( const AABB& aabb )
///
/// \brief	Union this box with the parameter. 
///
/// \param	aabb	The aabb. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE void AABB::unionWith( const AABB& aabb )
{
	m_MinExtent = Math::Min( m_MinExtent, aabb.m_MinExtent );
	m_MaxExtent = Math::Max( m_MaxExtent, aabb.m_MaxExtent );
}

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE void AABB::intersectWith( const AABB& aabb )
///
/// \brief	This box becomes the intersection of the param and itself. 
///
/// \param	aabb	The aabb. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE void AABB::intersectWith( const AABB& aabb )
{
	m_MinExtent = Math::Max( m_MinExtent, aabb.m_MinExtent );
	m_MaxExtent = Math::Min( m_MaxExtent, aabb.m_MaxExtent );
}


///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE bool AABB::intersects( const AABB& aabb ) const
///
/// \brief	Intersects. 
///
/// \todo	Fill in detailed method description. 
///
/// \param	aabb	The aabb. 
///
/// \return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE bool AABB::intersects( const AABB& aabb ) const
{
	const Math::Vector3 minI = Math::Max( m_MinExtent, aabb.m_MinExtent );
	const Math::Vector3 maxI = Math::Min( m_MaxExtent, aabb.m_MaxExtent );

	// TODO replace with simd compare and collapse result
	return( (minI[0] < maxI[0]) && (minI[1] < maxI[1]) && (minI[2] < maxI[2]) );
}


#if PLATFORM != GPU

///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE void AABB::drawDebug( const Core::Colour& colour,
/// const Math::Matrix4x4& trans ) const
///
/// \brief	Draw debug. 
///
/// \todo	Fill in detailed method description. 
///
/// \param	colour	The colour. 
/// \param	trans	The trans. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE void AABB::drawDebug( const Core::Colour& colour, const Math::Matrix4x4& trans ) const {
	g_pDebugRender->worldBox( colour, getBoxCenter(), Math::IdentityQuat(), getHalfLength()*2, trans );
}
#endif
