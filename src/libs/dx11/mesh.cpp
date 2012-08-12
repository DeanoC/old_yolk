//!-----------------------------------------------------
//!
//! \file mesh.cpp
//! a mesh
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "graphics.h"
#include "material.h"
#include "core/resourceman.h"
#include "rendercamera.h"
#include "rendermatrixcache.h"
#include "rendercontext.h"
#include "gpubvh.h"
#include "mesh.h"

namespace Graphics {
Mesh::Mesh() :
	m_MeshHandle(0),
	m_SimpleTransformNode( m_TransformMatrix ),
	Renderable( &m_SimpleTransformNode ) {
}

Mesh::Mesh( const char* pFilename ) :
	m_MeshHandle( Graphics::WobResourceHandle::Load( pFilename ) ),
	m_SimpleTransformNode( m_TransformMatrix ),
	Renderable( &m_SimpleTransformNode ) {

	// for now 
	WobResourcePtr pWob = m_MeshHandle->Acquire();
	localAabb = Core::AABB( pWob->m_spHeader->minAABB, pWob->m_spHeader->maxAABB );

}

Mesh::~Mesh() {
	if( m_MeshHandle ) {
		Core::ResourceMan::Get()->CloseResource( m_MeshHandle );
	}
}

void Mesh::Render( RenderContext* context, const Core::string& renderPassName ) {
	// set the prev WVP stored last frame to into the matrix cache
	context->matrixCache->setPreviousWorldViewProjectionMatrix( m_prevWVP );

	// change our world matrix
	context->matrixCache->setWorldMatrix( getTransformNode()->getWorldTransform() );
	// grap WVP for next frame (will cause a re-evail of WVP this frame)
	m_prevWVP = context->matrixCache->getMatrix( RenderMatrixCache::WORLD_VIEW_PROJECTION );

	WobResourcePtr pWob = m_MeshHandle->Acquire();

	// for now simple go through every material in turn rendering it
	for( uint32_t i = 0; i < pWob->m_spHeader->uiNumMaterials;++i) {
		Material& pMat = pWob->m_saMaterials[ i ];
		pMat.Render( context, renderPassName );
	}
}

void Mesh::DebugDraw( RenderContext* context ) {
	Renderable::DebugDraw( context );

//	WobResourcePtr pWob = m_MeshHandle->Acquire();
//	if(pWob->m_bvh) {
//		pWob->m_bvh->DebugDraw( &m_SimpleTransformNode, context->viewFrustum );
//	}

}


}