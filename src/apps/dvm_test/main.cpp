#include "core/core.h"
#include "core/clock.h"
#include "core/sysmsg.h"

#define START_FULLSCREEN	false
#define START_WIDTH			1280/2
#define START_HEIGHT		368 

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
void MainLoop();

bool g_bQuitFlag = false;
void QuitCallback() {
	g_bQuitFlag = true;
}
int g_iDebugMode = 0;
void DebugModeCallback( int debugMode ) {
	g_iDebugMode = debugMode;
}

int Main() {
	using namespace Core;

	SystemMessage::Get()->RegisterQuitCallback( QuitCallback );
	SystemMessage::Get()->RegisterDebugModeChangeCallback( DebugModeCallback );

	InitWindow( START_WIDTH, START_HEIGHT, START_FULLSCREEN );

	MainLoop();

	return 0;
}

void MainLoop() {
	using namespace Core;
	// clear the timer delta's
	Clock::Get()->update();
	Clock::Get()->update();

	// Main loop
	while( !g_bQuitFlag ) {
		Core::HouseKeep();
	}
	
}