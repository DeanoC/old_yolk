//!-----------------------------------------------------
//!
//! \file mesh.cpp
//! a mesh
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/resourceman.h"
#include "rendercontext.h"
#include "pipeline.h"
#include "mesh.h"

namespace Scene {

Mesh::Mesh() :
	meshHandle(0),
	ownedMatrix( CORE_NEW Math::Matrix4x4() ),
	Renderable( CORE_NEW Core::TransformNode( *ownedMatrix ) ) {
}

Mesh::Mesh( const char* pFilename ) :
	meshHandle( WobHandle::load( pFilename, NULL, Core::RMRF_NONE ) ),
	ownedMatrix( CORE_NEW Math::Matrix4x4() ),
	Renderable( CORE_NEW Core::TransformNode( *ownedMatrix ) ) {

	// for now 
	WobPtr wob = meshHandle->tryAcquire();
	if( wob ) {
		localAabb = Core::AABB( wob->header->minAABB, wob->header->maxAABB );
	}
}
Mesh::Mesh( const char* pFilename, Core::TransformNode* node ) :
	meshHandle( WobHandle::load( pFilename, NULL, Core::RMRF_NONE ) ),
	ownedMatrix( nullptr ),
	Renderable( node ) {

	// for now TODO on ready callback to set the localAabb before rendering
	WobPtr wob = meshHandle->acquire();
	if( wob ) {
		localAabb = Core::AABB( wob->header->minAABB, wob->header->maxAABB );
	}
}


Mesh::~Mesh() {
	if( meshHandle ) {
		Core::ResourceMan::get()->closeResource( meshHandle );
	}
	if( ownedMatrix ) {
		CORE_DELETE ownedMatrix;
		CORE_DELETE transformNode;
	}
}

void Mesh::render(  RenderContext* _context, const Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix ) const {

	// set the prev WVP stored last frame to into the constant cache
	// and change our world matrix
	_context->getConstantCache().setObject( prevWVP, _renderMatrix );

	// grap WVP for next frame (will cause a re-evail of WVP this frame)
	prevWVP = _context->getConstantCache().getMatrix( CVN_WORLD_VIEW_PROJ );

	WobPtr wob = meshHandle->tryAcquire();
	if( wob ) {
		wob->pipelineDataStores[ _pipeline->getIndex() ]->render( _context );
	} else {
//		LOG(INFO) << "Mesh not ready yet\n";
	}
}
void Mesh::renderTransparent( RenderContext* _context, const Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix ) const {

	// set the prev WVP stored last frame to into the constant cache
	// and change our world matrix
	_context->getConstantCache().setObject( prevWVP, _renderMatrix );

	// grap WVP for next frame (will cause a re-evail of WVP this frame)
	prevWVP = _context->getConstantCache().getMatrix( CVN_WORLD_VIEW_PROJ );

	WobPtr wob = meshHandle->tryAcquire();
	if( wob ) {
		wob->pipelineDataStores[ _pipeline->getIndex() ]->renderTransparent( _context );
	} else {
//		LOG(INFO) << "Mesh not ready yet\n";
	}
}

}