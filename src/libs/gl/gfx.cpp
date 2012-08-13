//!-----------------------------------------------------
//!
//! \file display.cpp
//! the main gl graphics singleton
//!
//!-----------------------------------------------------

#include "ogl.h"
#include "core/resourceman.h"
#include "scene/debugprims.h"
#include "scene/imagecomposer.h"

#include "shaderman.h"
#include "rendercontext.h"
#include "debugpipeline.h"
#include "vtpipeline.h"
#include "resourceloader.h"
#include "screen.h"

#include "gfx.h"

namespace Gl {

Gfx::Gfx() {
	valid = createGLContext();
	if( valid ) {
		valid = setGlPixelFormat( 0 ); // TODO stereo
	}
	if( valid ) {
		createRenderContexts();

		// some logging
	#if 1
		int gsAtomicCounter = 0;
		int fsAtomicCounter = 0;
		glGetIntegerv( GL_MAX_GEOMETRY_ATOMIC_COUNTERS, &gsAtomicCounter );
		glGetIntegerv( GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &fsAtomicCounter );
		LOG(INFO) << "Geometry Shader atomics : " << gsAtomicCounter;
		LOG(INFO) << "Fragment Shader atomics : " << fsAtomicCounter;
		int samples;
		glGetIntegerv( GL_MAX_SAMPLES, &samples);
		LOG(INFO) << "MSAA samples : " << samples;
	}
}

Gfx::~Gfx() {
}

Scene::ScreenPtr Gfx::createScreen( uint32_t _width, uint32_t _height, uint32_t _flags ) {
	using namespace Scene;
	if( !valid ) return Scene::ScreenPtr();


	#endif

	resourceLoader.reset( CORE_NEW ResourceLoader() );
	resourceLoader->installResourceTypes();

	// first render context is the main threads to abuse
	renderContexts[ RENDER_CONTEXT ]->threadActivate();
	renderContexts[ RENDER_CONTEXT ]->prepToRender();

	shaderMan.reset( CORE_NEW ShaderMan() );
	shaderMan->initDefaultPrograms();

	// install pipelines
	pipelines.push_back( std::unique_ptr<Scene::Pipeline>( CORE_NEW DebugPipeline(pipelines.size()) ) );
//	pipelines.push_back( std::unique_ptr<Scene::Pipeline>( CORE_NEW SceneCapturePipeline(pipelines.size()) ) );
//	pipelines.push_back( std::unique_ptr<Scene::Pipeline>( CORE_NEW VtPipeline(pipelines.size()) ) );
	pipelines.push_back( std::unique_ptr<Scene::Pipeline>( CORE_NEW DebugPipeline(pipelines.size()) ) );

	// add pipelines into a fast name lookup system
	for( size_t i = 0; i < pipelines.size(); ++i ) {
		hashPipeline[ pipelines[i]->getName() ] = i;
	}

	auto screen = std::make_shared<Gl::Screen>();
	screen->width = _width;
	screen->height = _height;
	screen->flags = _flags; 
	screen->renderer = this;
	// TODO stero
	if( screen->flags & SCRF_DEBUGPRIMS ) {
		screen->debugPrims.reset( CORE_NEW Scene::DebugPrims() );
	}
	if( screen->flags & SCRF_OVERLAY ) {
		screen->imageComposer.reset( CORE_NEW Scene::ImageComposer() );
	}
	return std::static_pointer_cast<Scene::Screen>( screen );
}

void Gfx::destroyScreen() {
	renderContexts.clear();
}
		
void Gfx::present( int backWidth, int backHeight ) {
	/*
	resourceLoader->showLoadingIfNeeded( finalComposer.get() );

	auto forId = hashPipeline[ "forward" ];
	auto vtId = hashPipeline[ "vt" ];

	// merge pipelines and display
	// bind, merge, unbind, this is needed to get debug market correct
	// and the keep the rule that each bind needs an unbind
//	pipelines[ forId ]->bind( &renderContexts[RENDER_CONTEXT] );
//	pipelines[ vtId ]->merge( &renderContexts[RENDER_CONTEXT] );
//	pipelines[ forId ]->unbind();
	pipelines[ vtId ]->display(&renderContexts[RENDER_CONTEXT],0,0 );

	// merge are allowed (and have to for MSAA) to fiddle with things
	// that binds set, so merge must be followed by unbind, but as we
	// want to draw stuff we have to call bind again
	pipelines[ forId ]->bind( &renderContexts[RENDER_CONTEXT] );
	finalComposer->render();
	debugPrims->flush();
	pipelines[ forId ]->unbind();
	renderContexts[ forId ].swapBuffers();

	frameCount++;
	totalPrimitiveCount = 0;
	*/
}

} // end Gl namespace