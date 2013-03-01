#include "core/core.h"
#if defined( USE_OPENGL )
#include "gl/ogl.h"
#include "gl/gfx.h"
#endif
#if defined( USE_DX11 )
#include "dx11/dx11.h"
#include "dx11/gfx.h"
#endif

#include "core/coreresources.h"
#include "scene/hier.h"
#include "scene/camera.h"
#include "scene/rendercontext.h"
#include "scene/debugpipeline.h"
#include "core/clock.h"
#include "core/sysmsg.h"
#include "core/development_context.h"
#include "localworld/debugcamcontext.h"
#include "localworld/inputhandlercontext.h"
#include "localworld/sceneworld.h"
#include "shell3d.h"

#define START_FLAGS			(SCRF_DEBUGPRIMS | SCRF_OVERLAY)
#define START_WIDTH			1280
#define START_HEIGHT		960

namespace {
	int s_screenWidth = START_WIDTH;
	int s_screenHeight = START_HEIGHT;
	int curWinWidth = s_screenWidth;
	int curWinHeight = s_screenHeight;

	bool s_quitFlag = false;
	int s_debugMode = 0;
}

void QuitCallback() {
	s_quitFlag = true;
}

void DebugModeCallback( int debugMode ) {
	s_debugMode = debugMode;
}

void ResizeCallback( int width, int height ) {
	curWinWidth = width;
	curWinHeight = height;
}

Shell3D::Shell3D() {
	world = std::make_shared<SceneWorld>();
}

Shell3D::~Shell3D() {
}

void Shell3D::start() {
	using namespace Core;
	using namespace Scene;

#if defined( USE_OPENGL )
	Gl::Gfx::init();
	renderer = static_cast< Renderer* >( Gl::Gfx::get() );
#elif defined( USE_DX11 )
	Dx11::Gfx::init();
	renderer = static_cast< Renderer* >( Dx11::Gfx::get() );
#else
	#error ASCII Renderer TODO
#endif

	InitWindow( s_screenWidth, s_screenHeight, !!(START_FLAGS & SCRF_FULLSCREEN) );
	SystemMessage::get()->registerQuitCallback( QuitCallback );
	SystemMessage::get()->registerDebugModeChangeCallback( DebugModeCallback );
	SystemMessage::get()->registerResizeCallback( ResizeCallback );

	screen = renderer->createScreen( s_screenWidth, s_screenHeight, START_FLAGS );
	if( !screen ) {
		LOG(ERROR) << "Unable to find adequate GPU";
		return;
	}

	renderer->addPipeline( std::make_shared<Scene::DebugPipeline>() );

}

void Shell3D::run() {
	using namespace Core;
	using namespace Scene;

	RenderContext* ctx = renderer->getPrimaryContext();
	// render context has been setup to use this thread by the same thread that created teh screen 

	// camera stuff is stuffed needs refactor
//	auto inputHandler = std::make_shared<InputHandlerContext>( world.get(), ctx );
//	DevelopmentContext::get()->addContext( "InputHandler",  inputHandler );
	auto debugCam = std::make_shared<DebugCamContext>( s_screenWidth, s_screenHeight, 90.0f, 0.1f, 5000.0f );
	DevelopmentContext::get()->addContext( "DebugCam",  debugCam );

	DevelopmentContext::get()->activateContext( "DebugCam" );

	// flush 'load' time from first time update
	Core::Clock shellClock;
	shellClock.update();

	// Main loop
	while( !s_quitFlag ) {

		float deltaT = shellClock.update();

		DevelopmentContext::get()->update( deltaT );
		world->update( deltaT );
		auto camera = DevelopmentContext::getr().getContext()->getCamera();
		if( camera ) {
			world->render( screen, "debug", camera, ctx );
		} else {
			// TODO here goes advanced camera systems
			TODO_ASSERT( camera );
		}

		DevelopmentContext::get()->display();
		world->displayRenderResults( screen, "debug", ctx );
		Core::HouseKeep();
	}	

}

void Shell3D::end() {
	using namespace Core;
	using namespace Scene;

//	renderer->destroyScreen();
#if defined( USE_OPENGL )
	Gl::Gfx::shutdown();	
#elif defined( USE_DX11 )
	Dx11::Gfx::shutdown();
#endif
}