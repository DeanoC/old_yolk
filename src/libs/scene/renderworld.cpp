//!-----------------------------------------------------
//!
//! \file renderworld.cpp
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/frustum.h"
#include "renderer.h"
#include "renderable.h"
#include "rendercontext.h"
#include "pipeline.h"
#include "screen.h"
#include "mesh.h"

#include "renderworld.h"

namespace Scene {

RenderWorld::RenderWorld() {
	renderCamera = std::make_shared<Scene::Camera>();
}

RenderWorld::~RenderWorld() {
}

uint32_t RenderWorld::addRenderable( RenderablePtr renderable ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	const auto it = renderables.push_back( renderable );
	visibleRenderables.reserve(renderables.size() + 1); // alloc now as its max size is renderables
	return (uint32_t)std::distance(renderables.begin(), it);
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

uint32_t RenderWorld::addRenderable2D( Renderable2DCallbackPtr renderable ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	const auto it = renderables2D.push_back( renderable );
	return (uint32_t) std::distance( renderables2D.begin(), it );
}

void RenderWorld::removeRenderable2D( Renderable2DCallbackPtr renderable ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	// currently the vector just grows, null entries and releasing memory but not actually
	// shriking the vector. This could be done by a stop the world mutex by for now probably okay as only 4/8 byte per renderable
	auto enIt = std::find(renderables2D.begin(), renderables2D.end(), renderable );
	assert( (enIt != renderables2D.end()) && "Renderable2D is not in this RenderWorld");
	*enIt = nullptr;
//	renderables.erase( enIt );
}

void RenderWorld::removeRenderable2D( uint32_t index ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	auto enIt = renderables2D.begin();
	std::advance( enIt, index );
	assert( (enIt != renderables2D.end()) && "Renderable2D is not in this RenderWorld");
	*enIt = nullptr;
}

void RenderWorld::determineVisibles( const std::shared_ptr<Scene::Camera>& camera ) {
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	// *copy* over the active camera, so if the game thread runs faster than the render one
	// we don't get screwed up graphics due to camera changing mid frame
	*renderCamera.get() = *camera.get();
	renderCamera->invalidate();

	// go through and cull renderables, also copy matrixes under
	// a lock to ensure other thread updates of renderables transform
	// isn't reflected to the renderer mid frame
	visibleRenderables.clear();

	// get all visible meshes
	RenderableContainer::const_iterator it = renderables.cbegin();
	for (const auto& r : renderables) {
		r->renderUpdate();
		r->getVisibleRenderablesOfType( renderCamera->getFrustum(), Scene::Renderable::ALL_TYPES, visibleRenderables );
	}
}

void RenderWorld::renderRenderables( RenderContext* context, Pipeline* pipeline ) {

	pipeline->bind( context );

	context->getConstantCache().setCamera( renderCamera );

	for( auto i = 0; i < pipeline->getGeomPassCount(); ++i ) {
		pipeline->startGeomPass( context, i );

		// output geometry
		STIndexContainer::const_iterator rmIt = visibleRenderables.cbegin();
		for (const auto& rm : visibleRenderables) {
			if( pipeline->isGeomPassOpaque( i ) ) {
				rm->render( context, pipeline, rm->getRenderMatrix() );
			} else {
				rm->renderTransparent( context, pipeline, rm->getRenderMatrix() );
			}
		}

		pipeline->endGeomPass( context, i );
	}
}

void RenderWorld::render( const ScreenPtr screen, const std::string& pipelineName, const std::shared_ptr<Scene::Camera> camera, RenderContext* context ) {
	Pipeline* pipeline = screen->getRenderer()->getPipeline( pipelineName );

	determineVisibles( camera );

	renderRenderables( context, pipeline );

	{
		Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );
		pipeline->start2DPass( context );
		for( const auto& it : renderables2D ) {
			(*it)( screen, context );
		}
		pipeline->end2DPass( context );
	}

	pipeline->resolve( context );

	if( debugRenderCallback ) {
		Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );
		debugRenderCallback();
	}
}

void RenderWorld::displayRenderResults( const ScreenPtr screen, const std::string& pipelineName, RenderContext* context ) {
	Pipeline* pipeline = screen->getRenderer()->getPipeline( pipelineName );
	context->getConstantCache().setCamera( renderCamera );
	screen->display( pipeline->getResult() );
}

}