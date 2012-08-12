//!-----------------------------------------------------
//!
//! \file rendermatrixcache.cpp
//!
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "graphics.h"
#include "rendercamera.h"
#include "rendermatrixcache.h"

namespace Graphics
{
RenderMatrixCache::RenderMatrixCache() :
	m_CachedFlags( 0 )
{
}

void RenderMatrixCache::setCamera( const RenderCameraPtr& camera )
{
	m_pCurrentCamera = camera;
	m_pCurrentCamera->m_bCached = false;
}
const RenderCameraPtr& RenderMatrixCache::getCamera() const {
	return m_pCurrentCamera;
}


void RenderMatrixCache::setWorldMatrix( const Math::Matrix4x4& worldMatrix )
{
	m_CachedFlags &= ~(	(1 << WORLD_INVERSE) | (1 << WORLD_INVERSE_TRANSPOSE) |
						(1 << WORLD_VIEW) | (1 << WORLD_VIEW_INVERSE) | (1 << WORLD_VIEW_INVERSE_TRANSPOSE) |
						(1 << WORLD_VIEW_PROJECTION) | (1 << WORLD_VIEW_PROJECTION_INVERSE) | (1 << WORLD_VIEW_PROJECTION_INVERSE_TRANSPOSE));
	m_MatrixCache[ WORLD ] = worldMatrix;
	m_BoneMatrixArray[0] = worldMatrix;
	m_CachedFlags |= (1 << WORLD);
}

void RenderMatrixCache::setPreviousWorldViewProjectionMatrix( const Math::Matrix4x4& wvpMatrix ) {
	m_MatrixCache[ PREVIOUS_WORLD_VIEW_PROJECTION ] = wvpMatrix;
	m_CachedFlags |= (1 << PREVIOUS_WORLD_VIEW_PROJECTION);
}



const Math::Matrix4x4& RenderMatrixCache::getMatrix( MATRIX_TYPE type )
{
	if( !m_pCurrentCamera )
	{
		// update our view and projection matrices from render camera
		m_MatrixCache[ VIEW ] = Math::IdentityMatrix();
		m_MatrixCache[ PROJECTION ] = Math::IdentityMatrix();
		// reset cached flags
		m_CachedFlags &= ~(	(1 << WORLD_VIEW) | (1 << WORLD_VIEW_INVERSE) | (1 << WORLD_VIEW_INVERSE_TRANSPOSE) |
							(1 << VIEW) | (1 << VIEW_INVERSE) | (1 << VIEW_INVERSE_TRANSPOSE) |
							(1 << VIEW_PROJECTION) | (1 << VIEW_PROJECTION_INVERSE) | (1 << VIEW_PROJECTION_INVERSE_TRANSPOSE) |
							(1 << PROJECTION) | (1 << PROJECTION_INVERSE) | (1 << PROJECTION_INVERSE_TRANSPOSE) |
							(1 << WORLD_VIEW_PROJECTION) | (1 << WORLD_VIEW_PROJECTION_INVERSE) | (1 << WORLD_VIEW_PROJECTION_INVERSE_TRANSPOSE));
		m_CachedFlags |= (1 << VIEW) | (1 << PROJECTION);
	} else
	{
		if( m_pCurrentCamera->m_bCached == false )
		{
			// update our view and projection matrices from render camera
			m_MatrixCache[ VIEW ] = m_pCurrentCamera->m_ViewMatrix;
			m_MatrixCache[ PROJECTION ] = m_pCurrentCamera->m_ProjectionMatrix;
			// reset cached flags
			m_CachedFlags &= ~(	(1 << WORLD_VIEW) | (1 << WORLD_VIEW_INVERSE) | (1 << WORLD_VIEW_INVERSE_TRANSPOSE) |
								(1 << VIEW) | (1 << VIEW_INVERSE) | (1 << VIEW_INVERSE_TRANSPOSE) |
								(1 << VIEW_PROJECTION) | (1 << VIEW_PROJECTION_INVERSE) | (1 << VIEW_PROJECTION_INVERSE_TRANSPOSE) |
								(1 << PROJECTION) | (1 << PROJECTION_INVERSE) | (1 << PROJECTION_INVERSE_TRANSPOSE) |
								(1 << WORLD_VIEW_PROJECTION) | (1 << WORLD_VIEW_PROJECTION_INVERSE) | (1 << WORLD_VIEW_PROJECTION_INVERSE_TRANSPOSE));

			m_CachedFlags |= (1 << VIEW) | (1 << PROJECTION);
			m_pCurrentCamera->m_bCached = true;
		}
	}

	// if the cache is valid just return
	if( m_CachedFlags & (1 << type) ) {
		return m_MatrixCache[ type ];
	} 

	// else recompute
	switch( type )
	{
	case VIEW_INVERSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::InverseMatrix( getMatrix(VIEW) );
	case VIEW_INVERSE_TRANSPOSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::TransposeMatrix( getMatrix(VIEW_INVERSE) );
	case PROJECTION_INVERSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::InverseMatrix( getMatrix(PROJECTION) );
	case PROJECTION_INVERSE_TRANSPOSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::TransposeMatrix( getMatrix(PROJECTION_INVERSE) );
	case VIEW_PROJECTION:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = getMatrix(VIEW) * getMatrix(PROJECTION) ;
	case VIEW_PROJECTION_INVERSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::InverseMatrix( getMatrix(VIEW_PROJECTION) );
	case VIEW_PROJECTION_INVERSE_TRANSPOSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::TransposeMatrix( getMatrix(VIEW_PROJECTION_INVERSE) );
	case WORLD:
		assert( false );
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ];
	case WORLD_INVERSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::InverseMatrix( getMatrix(WORLD) );
	case WORLD_INVERSE_TRANSPOSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::TransposeMatrix( getMatrix(WORLD_INVERSE) );
	case WORLD_VIEW:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = getMatrix(WORLD) * getMatrix(VIEW);	
	case WORLD_VIEW_INVERSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::InverseMatrix( getMatrix(WORLD_VIEW) );
	case WORLD_VIEW_INVERSE_TRANSPOSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::TransposeMatrix( getMatrix(WORLD_VIEW_INVERSE) );
	case WORLD_VIEW_PROJECTION:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = getMatrix(WORLD_VIEW) * getMatrix(PROJECTION);
	case WORLD_VIEW_PROJECTION_INVERSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::InverseMatrix( getMatrix(WORLD_VIEW_PROJECTION) );
	case WORLD_VIEW_PROJECTION_INVERSE_TRANSPOSE:
		m_CachedFlags |= (1 << type);
		return m_MatrixCache[ type ] = Math::TransposeMatrix( getMatrix(WORLD_VIEW_PROJECTION_INVERSE) );
	default:
		assert( false && ("Invalid matrix type") );
		return m_MatrixCache[ type ];
	};
}

}

