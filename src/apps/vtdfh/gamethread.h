#ifndef YOLK_VTDFH_GAMETHREAD_H_
#define YOLK_VTDFH_GAMETHREAD_H_ 1

class GameThread {
public:
	GameThread( SceneWorldPtr _world );
	~GameThread();
protected:
	Core::thread*		gameThread;
};
#endif