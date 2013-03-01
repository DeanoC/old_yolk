#include "vtdfh.h"
#include "shell3d.h"
#include "gamethread.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"

#include "levels/testlvl.h"

int Main() {
	using namespace Core;
	using namespace Scene;

	Shell3D shell;
	shell.start();
	SceneWorldPtr world = shell.getSceneWorld();

	auto inputHandler = std::make_shared<InputHandlerContext>();
	DevelopmentContext::get()->addContext( "InputHandler",  inputHandler );
	DevelopmentContext::get()->activateContext( "DebugCam" );

	auto lvl = std::make_shared<TestLvl>( world );
	lvl->spawnPlayer( 0 );

	auto gameThread = std::make_shared<GameThread>( world );
	shell.run();
	
	gameThread = nullptr;
	lvl = nullptr;

	shell.end();

	return 0;
}
