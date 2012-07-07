//!-----------------------------------------------------
//!
//! \file mesh.cpp
//! a mesh
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/resourceman.h"
#if RENDER_BACKEND == Gl
#include "gl/gl.h"
#include "gl/rendercontext.h"
#endif
#include "mesh.h"

namespace Scene {

Mesh::Mesh() :
	meshHandle(0),
	simpleTransformNode( transformMatrix ),
	Renderable( &simpleTransformNode ) {
}

Mesh::Mesh( const char* pFilename ) :
	meshHandle( WobResourceHandle::load( pFilename, NULL, Core::RMRF_NONE ) ),
	simpleTransformNode( transformMatrix ),
	Renderable( &simpleTransformNode ) {

	// for now 
	WobResourcePtr wob = meshHandle->tryAcquire();
	if( wob ) {
		localAabb = Core::AABB( wob->header->minAABB, wob->header->maxAABB );
	}
}

Mesh::~Mesh() {
	if( meshHandle ) {
		Core::ResourceMan::get()->closeResource( meshHandle );
	}
}

void Mesh::render( RenderContext* rc, const int pipelineName ) {
	RENDER_BACKEND::RenderContext* context = (RENDER_BACKEND::RenderContext*) rc;

	// set the prev WVP stored last frame to into the constant cache
	// and change our world matrix
	context->getConstantCache().changeObject( prevWVP, getTransformNode()->getWorldTransform() );

	// grap WVP for next frame (will cause a re-evail of WVP this frame)
	prevWVP = context->getConstantCache().getMatrix( RENDER_BACKEND::CVN_WORLD_VIEW_PROJ );

	WobResourcePtr wob = meshHandle->tryAcquire();
	if( wob ) {
		localAabb = Core::AABB( wob->header->minAABB, wob->header->maxAABB );
		wob->backEnd->pipelineDataStores[ pipelineName ]->render( context );
	} else {
//		LOG(INFO) << "Mesh not ready yet\n";
	}
}

void Mesh::debugDraw( RenderContext* rc ) const {
	Renderable::debugDraw( rc );

//	WobResourcePtr pWob = m_MeshHandle->acquire();
//	if(pWob->m_bvh) {
//		pWob->m_bvh->DebugDraw( &m_SimpleTransformNode, context->viewFrustum );
//	}

}

}