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
#include "scene/hie.h"
#include "scene/camera.h"
#include "scene/rendercontext.h"
#include "scene/debugpipeline.h"
#include "core/clock.h"
#include "core/sysmsg.h"
#include "core/development_context.h"
#include "debugcamcontext.h"
#include "inputhandlercontext.h"
#include "clientworld.h"
#include "client.h"

#define START_FLAGS			(SCRF_DEBUGPRIMS | SCRF_OVERLAY)
#define START_WIDTH			1024
#define START_HEIGHT		1024

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

DwmClient::DwmClient() {
	world = std::make_shared<ClientWorld>();
}

DwmClient::~DwmClient() {
}

void DwmClient::start() {
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

void DwmClient::run() {
	using namespace Core;
	using namespace Scene;

	RenderContext* ctx = renderer->getPrimaryContext();
	// render context has been setup to use this thread by the same thread that created teh screen 

	// camera stuff is stuffed needs refactor
//	auto inputHandler = std::make_shared<InputHandlerContext>( world.get(), ctx );
//	DevelopmentContext::get()->addContext( "InputHandler",  inputHandler );
	auto debugCam = std::make_shared<DebugCamContext>( world.get(), ctx, s_screenWidth, s_screenHeight, 90.0f, 0.1f, 5000.0f );
	DevelopmentContext::get()->addContext( "DebugCam",  debugCam );

	DevelopmentContext::get()->activateContext( "DebugCam" );

	// flush 'load' time from first time update
	Clock::get()->update();

	// Main loop
	while( !s_quitFlag ) {

		float deltaT = Clock::get()->update();

		DevelopmentContext::get()->update( deltaT );

		world->render( screen, "debug", ctx );

		DevelopmentContext::get()->display();
		world->displayRenderResults( screen, "debug", ctx );
		Core::HouseKeep();
	}	

}

void DwmClient::end() {
	using namespace Core;
	using namespace Scene;

//	renderer->destroyScreen();
#if defined( USE_OPENGL )
	Gl::Gfx::shutdown();	
#elif defined( USE_DX11 )
	Dx11::Gfx::shutdown();
#endif
}