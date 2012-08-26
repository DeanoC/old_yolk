#include "vtdfh.h"
#include "gamethread.h"
#include "core/clock.h"
#include "core/development_context.h"
extern bool g_quitFlag;

std::atomic<bool>	shutdownFinished;

GameThread::GameThread( SceneWorldPtr _world ) {
	SceneWorldPtr	world = _world;
	gameThread = CORE_NEW Core::thread( [world] {
			// flush 'load' time from first time update
			shutdownFinished = false;
			Core::Clock::get()->update();
			while( !g_quitFlag ) {
				float deltaT = Core::Clock::get()->update();
				Core::DevelopmentContext::get()->update( deltaT );
				world->update( deltaT );
			}
			shutdownFinished = true;
	} );
}

GameThread::~GameThread() {
	while( shutdownFinished == false ){}
	gameThread->join();
	CORE_DELETE gameThread;
}