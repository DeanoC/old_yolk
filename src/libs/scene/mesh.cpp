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

	// for now 
	WobPtr wob = meshHandle->tryAcquire();
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

void Mesh::render( RenderContext* context, Pipeline* pipeline ) {

	// set the prev WVP stored last frame to into the constant cache
	// and change our world matrix
	context->getConstantCache().setObject( prevWVP, getTransformNode()->getRenderMatrix() );

	// grap WVP for next frame (will cause a re-evail of WVP this frame)
	prevWVP = context->getConstantCache().getMatrix( CVN_WORLD_VIEW_PROJ );

	WobPtr wob = meshHandle->tryAcquire();
	if( wob ) {
		localAabb = Core::AABB( wob->header->minAABB, wob->header->maxAABB );
		wob->pipelineDataStores[ pipeline->getIndex() ]->render( context );
	} else {
//		LOG(INFO) << "Mesh not ready yet\n";
	}
}
void Mesh::renderTransparent( RenderContext* context, Pipeline* pipeline ) {

	// set the prev WVP stored last frame to into the constant cache
	// and change our world matrix
	context->getConstantCache().setObject( prevWVP, getTransformNode()->getRenderMatrix() );

	// grap WVP for next frame (will cause a re-evail of WVP this frame)
	prevWVP = context->getConstantCache().getMatrix( CVN_WORLD_VIEW_PROJ );

	WobPtr wob = meshHandle->tryAcquire();
	if( wob ) {
		localAabb = Core::AABB( wob->header->minAABB, wob->header->maxAABB );
		wob->pipelineDataStores[ pipeline->getIndex() ]->renderTransparent( context );
	} else {
//		LOG(INFO) << "Mesh not ready yet\n";
	}
}

}