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

#include "renderworld.h"

namespace Scene {

RenderWorld::RenderWorld() {
	renderCamera = std::make_shared<Scene::Camera>();
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
	Core::AABB waabb;

	Scene::Renderable* gatherArray[100];
	// get all meshes
	RenderableContainer::const_iterator it = renderables.cbegin();
	while( it != renderables.cend() ) {
		auto meshCount = (*it)->getActualRenderablesOfType( Renderable::R_MESH, 100, gatherArray );
		for( unsigned int i = 0;i < meshCount; ++i ) {
			gatherArray[i]->getTransformNode()->setRenderMatrix();
			gatherArray[i]->getRenderAABB( waabb );
			if( renderCamera->getFrustum().cullAABB( waabb ) != Core::Frustum::OUTSIDE ) {
				visibleRenderables.push_back( gatherArray[i] );
			}
		}
		++it;
	}
}

void RenderWorld::renderRenderables( RenderContext* context, Pipeline* pipeline ) {

	pipeline->bind( context );

	context->getConstantCache().setCamera( renderCamera );

	for( auto i = 0; i < pipeline->getGeomPassCount(); ++i ) {
		pipeline->startGeomPass( context, i );

		// output geometry
		STIndexContainer::const_iterator rmit = visibleRenderables.cbegin();
		while( rmit != visibleRenderables.cend() ) {
			if( pipeline->isGeomPassOpaque( i ) ) {
				(*rmit)->render( context, pipeline );
			} else {
				(*rmit)->renderTransparent( context, pipeline );
			}
			++rmit;
		}

		pipeline->endGeomPass( context, i );
	}


	pipeline->unbind( context );
}

void RenderWorld::render( const ScreenPtr screen, const std::string& pipelineName, const std::shared_ptr<Scene::Camera> camera, RenderContext* context ) {
	Pipeline* pipeline = screen->getRenderer()->getPipeline( pipelineName );

	determineVisibles( camera );

	renderRenderables( context, pipeline );
	if( debugRenderCallback ) {
		debugRenderCallback();
	}
}

void RenderWorld::displayRenderResults( const ScreenPtr screen, const std::string& pipelineName, RenderContext* context ) {
	Pipeline* pipeline = screen->getRenderer()->getPipeline( pipelineName );
	context->getConstantCache().setCamera( renderCamera );
	screen->display( pipeline->getResult() );
}

}