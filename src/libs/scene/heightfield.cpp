//!-----------------------------------------------------
//!
//! \file heightfield.cpp
//! a heightfield
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/resourceman.h"
#if RENDER_BACKEND == Gl
#include "gl/gl.h"
#include "gl/rendercontext.h"
#endif
#include "heightfield.h"

namespace Scene {

Heightfield::Heightfield() :
//	textureHandle(0),
	simpleTransformNode( transformMatrix ),
	Renderable( &simpleTransformNode ) {
}

Heightfield::Heightfield( const char* pFilename ) :
//	textureHandle( TextureHandle::load( pFilename, NULL, Core::RMRF_NONE ) ),
	simpleTransformNode( transformMatrix ),
	Renderable( &simpleTransformNode ) {

	// for now 
//	TexturePtr tex = textureHandle->tryAcquire();
	// TODO AABB
}

Heightfield::~Heightfield() {
//	if( textureHandle ) {
//		Core::ResourceMan::get()->closeResource( textureHandle );
//	}
}

void Heightfield::render( RenderContext* rc, const int pipelineName ) {
	RENDER_BACKEND::RenderContext* context = (RENDER_BACKEND::RenderContext*) rc;

	// set the prev WVP stored last frame to into the constant cache
	// and change our world matrix
	context->getConstantCache().changeObject( prevWVP, getTransformNode()->getRenderMatrix() );

	// grap WVP for next frame (will cause a re-evail of WVP this frame)
	prevWVP = context->getConstantCache().getMatrix( RENDER_BACKEND::CVN_WORLD_VIEW_PROJ );

//	TexturePtr tex = textureHandle->tryAcquire();
//	if( tex ) {
//	} else {
//	}
}

void Heightfield::debugDraw( RenderContext* rc ) const {
	Renderable::debugDraw( rc );

//	WobResourcePtr pWob = m_MeshHandle->acquire();
//	if(pWob->m_bvh) {
//		pWob->m_bvh->DebugDraw( &m_SimpleTransformNode, context->viewFrustum );
//	}

}

}