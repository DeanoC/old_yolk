#include "core/core.h"
#include "gl/ogl.h"
#include "cl/ocl.h"
#include "gl/gfx.h"
#include "cl/platform.h"
#include "core/coreresources.h"
#include "scene/hie.h"
#include "scene/camera.h"
#include "scene/rendercontext.h"
#include "core/clock.h"
#include "core/sysmsg.h"
#include "core/development_context.h"
#include "debugcamcontext.h"
#include "inputhandlercontext.h"
#include "clientworld.h"
#include "client.h"

#define START_FULLSCREEN	false
#define START_WIDTH			1024
#define START_HEIGHT		1024
#define START_AA			AA_NONE

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

void DwmClient::run() {
	using namespace Core;
	using namespace Scene;
	using namespace Gl;

	Gfx::init();
	if( !Cl::Platform::exists() )
		Cl::Platform::init();


	InitWindow( s_screenWidth, s_screenHeight, START_FULLSCREEN );
	bool glOk = Gfx::get()->createScreen( s_screenWidth, s_screenHeight, START_FULLSCREEN, Gl::Gfx::START_AA );
	if( glOk == false ) {
		LOG(ERROR) << "GL unable to find adequate GPU";
		return;
	}
	SystemMessage::get()->registerQuitCallback( QuitCallback );
	SystemMessage::get()->registerDebugModeChangeCallback( DebugModeCallback );
	SystemMessage::get()->registerResizeCallback( ResizeCallback );

	Scene::RenderContext* ctx = (Scene::RenderContext*) Gfx::get()->getThreadRenderContext( Gfx::RENDER_CONTEXT );
	ctx->threadActivate();
	ctx->prepToRender();

	auto inputHandler = std::make_shared<InputHandlerContext>( world.get(), ctx );
	DevelopmentContext::get()->addContext( "InputHandler",  inputHandler );

	auto debugCam = std::make_shared<DebugCamContext>( world.get(), ctx, s_screenWidth, s_screenHeight, 90.0f, 0.1f, 5000.0f );
	DevelopmentContext::get()->addContext( "DebugCam",  debugCam );

	DevelopmentContext::get()->activateContext( "InputHandler" );

	// flush 'load' time from first time update
	Clock::get()->update();

	// Main loop
	while( !s_quitFlag ) {

		float deltaT = Clock::get()->update();

		DevelopmentContext::get()->update( deltaT );

		world->render( ctx );

		DevelopmentContext::get()->display();

		Gl::Gfx::get()->present( curWinWidth, curWinHeight );
		Core::HouseKeep();
	}	

	Gfx::get()->shutdownScreen();
	Gfx::shutdown();	
}