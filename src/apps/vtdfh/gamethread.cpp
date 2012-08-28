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
				Core::Clock::sleep( Math::Clamp(1.0f/120.0f - deltaT,0.0f,1.0f) );
			}
			shutdownFinished = true;
	} );
}

GameThread::~GameThread() {
	while( shutdownFinished == false ){}
	gameThread->join();
	CORE_DELETE gameThread;
}