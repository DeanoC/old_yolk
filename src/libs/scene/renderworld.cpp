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

RenderWorld::RenderWorld() :
	activeCamera( nullptr ) {
	renderCamera = std::make_shared<Scene::Camera>();
	debugCamera = std::make_shared<Scene::Camera>();
}

RenderWorld::~RenderWorld() {
}

uint32_t RenderWorld::addRenderable( RenderablePtr renderable ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );
	auto it = renderables.push_back( renderable );
	const uint32_t index = (uint32_t) std::distance( renderables.begin(), it );
	visibleRenderables.reserve( index + 1 );

	return index;
}

void RenderWorld::removeRenderable( RenderablePtr renderable ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	// currently the vector just grows, null entries and releasing memory but not actually
	// shriking the vector. This could be done by a stop the world mutex by for now probably okay as only 4/8 byte per renderab le
	RenderableContainer::iterator enIt = std::find(renderables.begin(), renderables.end(), renderable );
	assert( (enIt != renderables.end()) && "Renderable is not in this RenderWorld");
	*enIt = nullptr;
//	renderables.erase( enIt );
}
void RenderWorld::removeRenderable( uint32_t index ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );
	RenderableContainer::iterator enIt = renderables.begin();
	std::advance( enIt, index );
	assert( (enIt != renderables.end()) && "Renderable is not in this RenderWorld");
	*enIt = nullptr;
}

uint32_t RenderWorld::addCamera( CameraPtr camera ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );
	auto it = cameras.push_back( camera );
	return (uint32_t) std::distance( cameras.begin(), it );
}

void RenderWorld::removeCamera( CameraPtr camera ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );
	// currently the vector just grows, null entries and releasing memory but not actually
	// shriking the vector. This could be done by a stop the world mutex by for now probably okay as only 4/8 byte per renderab le
	CameraContainer::iterator enIt = std::find(cameras.begin(), cameras.end(), camera );
	assert( (enIt != cameras.end()) && "Camera is not in this RenderWorld");
	*enIt = nullptr;
//	renderables.erase( enIt );
}
void RenderWorld::removeCamera( uint32_t index ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );
	CameraContainer::iterator enIt = cameras.begin();
	std::advance( enIt, index );
	assert( (enIt != cameras.end()) && "Camera is not in this RenderWorld");
	*enIt = nullptr;
}


void RenderWorld::renderRenderables( RenderContext* context, const size_t pipelineIndex ) {
	using namespace RENDER_BACKEND;
	Gfx* gfx = Gfx::get();

	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	// *copy* over the active camera, so if the game thread runs faster than the render one
	// we don't get screwed up graphics due to camera changing mid frame
	if( activeCamera ) {
		*renderCamera.get() = *activeCamera.get();
	}
	context->setCamera( renderCamera );
	Pipeline* pipeline = gfx->getPipeline( pipelineIndex );
	pipeline->bind( context );

	// first go through and cull renderables, also copy matrixes under
	// a lock to ensure other thread updates of renderables transform
	// isn't reflected to the renderer mid frame
	visibleRenderables.clear();
	Core::AABB waabb;

	RenderableContainer::const_iterator it = renderables.cbegin();
	while( it != renderables.cend() ) {
		const RenderablePtr& toRender = (*it);
		const uint32_t index = (uint32_t) 
				std::distance( renderables.cbegin(), it );

		toRender->getTransformNode()->setRenderMatrix();
		toRender->getWorldAABB( waabb );
//			if( activeCamera &&
//				activeCamera->getFrustum().cullAABB( waabb ) != 
//								Core::Frustum::OUTSIDE ) 
		{
			visibleRenderables.push_back( index );
		}

		++it;
	}
	updateLock.unlock();

	for( auto i = 0; i < pipeline->getGeomPassCount(); ++i ) {
		pipeline->startGeomPass( i );

		// output geometry
		STIndexContainer::const_iterator rmit = visibleRenderables.cbegin();
		while( rmit != visibleRenderables.cend() ) {
			const RenderablePtr& toRender = renderables[ (*rmit) ];
			toRender->render( context, pipelineIndex );
			++rmit;
		}

		pipeline->endGeomPass( i );
	}

	pipeline->unbind();
}
void RenderWorld::debugDraw( RenderContext* context ) {
	return;
	using namespace RENDER_BACKEND;
	Gfx* gfx = Gfx::get();

	if( activeCamera ) {
		*debugCamera.get() = *activeCamera.get();
	}
	context->setCamera( debugCamera );

	Pipeline* pipeline = gfx->getPipeline( 0 );
	pipeline->bind( context );

	Core::AABB waabb;
	RenderableContainer::const_iterator it = renderables.begin();
	while( it != renderables.end() ) {
		const RenderablePtr& toRender = (*it);
		toRender->getWorldAABB( waabb );
//		if( debugCamera &&
//			debugCamera->getFrustum().cullAABB( waabb ) != 
//							Core::Frustum::OUTSIDE ) 
		{
			toRender->debugDraw( context );
		}
		++it;
	}

	pipeline->unbind();

}

void RenderWorld::render( RenderContext* context ) {
	renderRenderables( context, 1 );
}

}