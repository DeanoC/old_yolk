//!-----------------------------------------------------
//!
//! \file renderworld.cpp
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "renderable.h"
#include "DeferredLightGBuffers11.h"
#include "gfx.h"
#include "rendermatrixcache.h"
#include "core/frustum.h"
#include "rendercontext.h"
#include "lightcontext.h"
#include "sm1view.h"
#include "renderworld.h"


namespace Dx11 {

RenderWorld::RenderWorld() : 
	gbuffers11(0) {

	switch( Gfx::Get()->getShaderModel() ) {
		case Gfx::SM5_0:
			gbuffers11 = CORE_NEW DeferredLightGBuffers11;
			break;
		default:
			break;
	}

	testSM = CORE_NEW SM1view();
}

RenderWorld::~RenderWorld() {
	CORE_DELETE testSM;
	CORE_DELETE gbuffers11;
}

void RenderWorld::addRenderable( Renderable* renderable ) {
	renderables.push_back( renderable );
}

void RenderWorld::removeRenderable( Renderable* renderable ) {
	RenderableContainer::iterator enIt = std::find(renderables.begin(), renderables.end(), renderable );
	assert( (enIt != renderables.end()) && "Renderable is not in this RenderWorld");
	renderables.erase( enIt );
}

void RenderWorld::renderRenderables( RenderContext* context, const Core::string& renderPassName ) {
	// TODO frustum cull once not each time RenderRenderables is called

	// output to current render targets
	Gfx::Get()->pushDebugMarker( renderPassName.c_str() );
	RenderableContainer::const_iterator it = renderables.begin();
	while( it != renderables.end() ) {
		Renderable* toRender = (*it);
		if( context->viewFrustum->cullAABB( toRender->getWorldAABB() ) != Core::Frustum::OUTSIDE ) {
			toRender->Render( context, renderPassName );
		}
		++it;
	}

	Gfx::Get()->popDebugMarker();
}
void RenderWorld::debugDraw( RenderContext* context ) {

	Gfx::Get()->pushDebugMarker( "RenderWorld::DebugDraw" );

	// output to geometry buffers
	RenderableContainer::const_iterator it = renderables.begin();
	while( it != renderables.end() ) {
		Renderable* toRender = (*it);
		if( context->viewFrustum->cullAABB( toRender->getWorldAABB() ) != Core::Frustum::OUTSIDE ) {
			toRender->DebugDraw( context );
		}
		++it;
	}

	Gfx::Get()->popDebugMarker();
}



void RenderWorld::render( RenderContext* context ) {

	if(gbuffers11) {
		render11( context );
	}
}

void RenderWorld::render11( RenderContext* context ) {
	gbuffers11->Start( context ); // make sure buffers are acquired


	// output to depth and result buffers (if cache entry is valid)
	gbuffers11->StartDepthAndReprojectPass();
	renderRenderables( context, "depthandreproject"  );

//	renderVisibilityMaps( context );

	// output to g-buffers
	gbuffers11->StartGBufferPass();
	renderRenderables( context, "dl" );

	// light g-buffers
	gbuffers11->Light( 0 );

//	testSM->debugDraw( context, gbuffers11->getResultUav() );

	// resolve to back buffer (tone map, MSAA resolve etc.)
	gbuffers11->Finish(); // no more access (read or write) to the gbuffers, so finish them
	Gfx::Get()->resetToDefaultRenderTargets();
	gbuffers11->Resolve();
}

void RenderWorld::renderVisibilityMaps( RenderContext* context ) {
	Gfx::Get()->pushDebugMarker( "spanbuffer_setup" );

	testSM->startRender(context);

	renderRenderables( context, "spanbuffer" );

	testSM->endRender(context);

	Gfx::Get()->popDebugMarker();
}

}