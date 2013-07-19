#ifndef YOLK_VTDFH_GAMETHREAD_H_
#define YOLK_VTDFH_GAMETHREAD_H_ 1

class GameThread {
public:
	GameThread( SceneWorldPtr _world );
	~GameThread();
	void exit() { exitFlag = true; }
protected:
	std::atomic<bool>	exitFlag;
	Core::thread*		gameThread;
	Core::thread*		inputThread;
};
#endif