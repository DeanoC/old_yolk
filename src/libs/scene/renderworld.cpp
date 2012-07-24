//!-----------------------------------------------------
//!
//! \file renderworld.cpp
//!
//!-----------------------------------------------------

#include "scene.h"
#include "renderable.h"
#include "core/frustum.h"
#include "rendercontext.h"
#if RENDER_BACKEND == Gl
#include "gl/gfx.h"
#elif RENDER_BACKEND == Dx11
#include "dx11/gfx.h"
#endif

#include "renderworld.h"

namespace Scene {

RenderWorld::RenderWorld() {
}

RenderWorld::~RenderWorld() {
}

uint32_t RenderWorld::addRenderable( RenderablePtr renderable ) {
	auto it = renderables.push_back( renderable );
	return (uint32_t) std::distance( renderables.begin(), it );
}

void RenderWorld::removeRenderable( RenderablePtr renderable ) {
	// currently the vector just grows, null entries and releasing memory but not actually
	// shriking the vector. This could be done by a stop the world mutex by for now probably okay as only 4/8 byte per renderab le
	RenderableContainer::iterator enIt = std::find(renderables.begin(), renderables.end(), renderable );
	assert( (enIt != renderables.end()) && "Renderable is not in this RenderWorld");
	*enIt = nullptr;
//	renderables.erase( enIt );
}
void RenderWorld::removeRenderable( uint32_t index ) {
	RenderableContainer::iterator enIt = renderables.begin();
	std::advance( enIt, index );
	assert( (enIt != renderables.end()) && "Renderable is not in this RenderWorld");
	*enIt = nullptr;
}

uint32_t RenderWorld::addCamera( CameraPtr camera ) {
	auto it = cameras.push_back( camera );
	return (uint32_t) std::distance( cameras.begin(), it );
}

void RenderWorld::removeCamera( CameraPtr camera ) {
	// currently the vector just grows, null entries and releasing memory but not actually
	// shriking the vector. This could be done by a stop the world mutex by for now probably okay as only 4/8 byte per renderab le
	CameraContainer::iterator enIt = std::find(cameras.begin(), cameras.end(), camera );
	assert( (enIt != cameras.end()) && "Camera is not in this RenderWorld");
	*enIt = nullptr;
//	renderables.erase( enIt );
}
void RenderWorld::removeCamera( uint32_t index ) {
	CameraContainer::iterator enIt = cameras.begin();
	std::advance( enIt, index );
	assert( (enIt != cameras.end()) && "Camera is not in this RenderWorld");
	*enIt = nullptr;
}


void RenderWorld::renderRenderables( RenderContext* context, const size_t pipelineIndex ) {
	using namespace RENDER_BACKEND;

	// TODO frustum cull once not each time RenderRenderables is called

	Gfx* gfx = Gfx::get();

	Pipeline* pipeline = gfx->getPipeline( pipelineIndex );
	pipeline->bind( context, true );
	
	// output to current render targets
	RenderableContainer::const_iterator it = renderables.begin();
	while( it != renderables.end() ) {
		const RenderablePtr& toRender = (*it);
//		if( context->viewFrustum->cullAABB( toRender->getWorldAABB() ) != Core::Frustum::OUTSIDE ) 
		{
			toRender->render( context, pipelineIndex );
		}
		++it;
	}

	pipeline->unbind( context );

}
void RenderWorld::debugDraw( RenderContext* context ) {

	context->pushDebugMarker( "RenderWorld::DebugDraw" );

	// output to geometry buffers
	RenderableContainer::const_iterator it = renderables.begin();
	while( it != renderables.end() ) {
		const RenderablePtr& toRender = (*it);
//		if( context->viewFrustum->cullAABB( toRender->getWorldAABB() ) != Core::Frustum::OUTSIDE ) 
		{
			toRender->debugDraw( context );
		}
		++it;
	}

	context->popDebugMarker(); 
}

void RenderWorld::render( RenderContext* context ) {
	using namespace RENDER_BACKEND;

	Gfx* gfx = Gfx::get();

	for( size_t i = 0; i < gfx->getNumPipelines(); ++i ) {
		// TODO how to select pipelines... hmmm
		if( i == 0 ) {
			renderRenderables( context, i );
		}
	}

}

}