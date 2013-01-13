#include "cyberhackerspace.h"
#include "gamethread.h"
#include "core/clock.h"
#include "core/development_context.h"
extern bool g_quitFlag;

#define INPUT_THREAD_HZ (1.0f/240.0f)
#define GAME_THREAD_HZ	(1.0f/120.0f)
#define SLOWEST_HZ		(1.0f/15.0f)

std::atomic<int>	shutdownFinished;

GameThread::GameThread( SceneWorldPtr _world ) {
	SceneWorldPtr	world = _world;
	shutdownFinished = 0;

	inputThread = CORE_NEW Core::thread( [world] {
		shutdownFinished++;
		Core::Clock inputClock;
		inputClock.update();			
		while( !g_quitFlag ) {
			float deltaT = inputClock.update();
			deltaT = Math::Min( deltaT, SLOWEST_HZ );
			Core::DevelopmentContext::get()->update( deltaT );
			Core::Clock::sleep( Math::Max(INPUT_THREAD_HZ - deltaT, 0.0f) );
		}
		shutdownFinished--;
	} );

	gameThread = CORE_NEW Core::thread( [world] {
		shutdownFinished++;
		Core::Clock gameClock;
		// flush 'load' time from first time update
		gameClock.update();			
		while( !g_quitFlag ) {
			float deltaT = gameClock.update();
			deltaT = Math::Min( deltaT, SLOWEST_HZ );
			world->update( deltaT );
			Core::Clock::sleep( Math::Max(GAME_THREAD_HZ - deltaT, 0.0f) );
		}
		shutdownFinished--;
	} );
}

GameThread::~GameThread() {
	while( shutdownFinished > 0 ){}
	gameThread->join();
	inputThread->join();
	CORE_DELETE gameThread;
	CORE_DELETE inputThread;
}