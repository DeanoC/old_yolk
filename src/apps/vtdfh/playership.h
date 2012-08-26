#ifndef YOLK_VTDFH_PLAYERSHIP_H_
#define YOLK_VTDFH_PLAYERSHIP_H_ 1

#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"

class ZarchCam;
class ObjectCam;

class PlayerShip : public Updatable {
public:
	PlayerShip( SceneWorldPtr _world, int _localPlayerNum );
	~PlayerShip();
	
	virtual void update( float timeS ) override;
private:
	bool findHeightBelow( float& height );

	SceneWorldPtr 	world;
	ThingPtr  		ship;
	int localPlayerNum;
	std::shared_ptr<InputHandlerContext>	inputContext;
	std::shared_ptr<ZarchCam>				zarchCam;
	std::shared_ptr<ObjectCam>				objectCam;

	float lastMouseX;
	float lastMouseY;

	float xRot;
	float yRot;
	float zRot;
	float speed;
	float angularSpeed;
	Math::Vector3 position;

	float curForwardMotion;
	float curSideMotion;
};

#endif