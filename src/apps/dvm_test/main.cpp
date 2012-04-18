#include "core/core.h"
#include "core/clock.h"
#include "core/sysmsg.h"
#include "dwm/dwm.h"

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

Core::shared_ptr<Dwm> dwm;

int Main() {
	using namespace Core;

	SystemMessage::Get()->RegisterQuitCallback( QuitCallback );
	SystemMessage::Get()->RegisterDebugModeChangeCallback( DebugModeCallback );

//	InitWindow( START_WIDTH, START_HEIGHT, START_FULLSCREEN );

	dwm.reset( new Dwm );
	dwm->bootstrapLocal();

//	MainLoop();

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