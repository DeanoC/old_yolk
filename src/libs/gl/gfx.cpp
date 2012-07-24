//!-----------------------------------------------------
//!
//! \file display.cpp
//! the main gl graphics singleton
//!
//!-----------------------------------------------------

#include "gl.h"
#include "core/resourceman.h"
#include "debugprims.h"
#include "shaderman.h"

#include "rendercontext.h"
#include "forwardpipeline.h"
#include "resourceloader.h"
#include "imagecomposer.h"


// compute renderer needs CL
#include "cl/cl.h"
#include "cl/platform.h"

#include "gfx.h"

namespace Gl {

Gfx::Gfx() {
	if( !Cl::Platform::exists() )
		Cl::Platform::init();
}

Gfx::~Gfx() {
}

bool Gfx::createScreen(	unsigned int iWidth, unsigned int iHeight, 
						bool bFullScreen, 
						ANTI_ALIASING aaSetting) {
	screenWidth = iWidth;
	screenHeight = iHeight;
	aaSetting = aaSetting;
	frameCount = 0;
	totalPrimitiveCount = 0;
	passPrimitiveCount = 0;

	if( createGLContext() == false )
		return false;

	createRenderContexts();

	using namespace Cl;
	// compute render needs to setup CL render to go
	if( !Cl::Platform::exists() )
		Cl::Platform::init();
	Cl::Platform::get()->createDevices();

	resourceLoader.reset( CORE_NEW ResourceLoader() );
	resourceLoader->installResourceTypes();

	// first render context is the main threads to abuse
	renderContexts[ RENDER_CONTEXT ].threadActivate();
	renderContexts[ RENDER_CONTEXT ].prepToRender();

	shaderMan.reset( CORE_NEW ShaderMan() );
	shaderMan->initDefaultPrograms();

	debugPrims.reset( CORE_NEW DebugPrims() );
	finalComposer.reset( CORE_NEW ImageComposer() );

	// install pipelines
	pipelines.push_back( std::unique_ptr<Scene::Pipeline>( CORE_NEW ForwardPipeline(pipelines.size()) ) );
//	pipelines.push_back( std::unique_ptr<Scene::Pipeline>( CORE_NEW SceneCapturePipeline(pipelines.size()) ) );

	// add pipelines into a fast name lookup system
	for( size_t i = 0; i < pipelines.size(); ++i ) {
		hashPipeline[ pipelines[i]->getName() ] = i;
	}

	// some logging
#if 1
	int gsAtomicCounter = 0;
	int fsAtomicCounter = 0;
	glGetIntegerv( GL_MAX_GEOMETRY_ATOMIC_COUNTERS, &gsAtomicCounter );
	glGetIntegerv( GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &fsAtomicCounter );
	LOG(INFO) << "Geometry Shader atomics : " << gsAtomicCounter;
	LOG(INFO) << "Fragment Shader atomics : " << fsAtomicCounter;
#endif


	return true;
}


void Gfx::shutdownScreen() {
	renderContexts.reset( 0 );
}

void Gfx::present( int backWidth, int backHeight ) {
	resourceLoader->showLoadingIfNeeded( finalComposer.get() );

	pipelines[0]->bind( &renderContexts[RENDER_CONTEXT], false );
	debugPrims->flush();
	finalComposer->render();
	pipelines[0]->unbind( &renderContexts[0] );

	pipelines[0]->display( &renderContexts[RENDER_CONTEXT], backWidth, backHeight );
	renderContexts[0].swapBuffers();

	frameCount++;
	totalPrimitiveCount = 0;
}


RenderContext* Gfx::getThreadRenderContext( THREAD_CONTEXT index ) const {
	return &renderContexts[index];
}

const std::string& Gfx::getShaderModelAsString( const SHADER_MODEL sm  ) { 
	static std::string smStrings[] = {
		"GL4_2",
	};
	return smStrings[ sm ] ; 
}

} // end Gl namespace