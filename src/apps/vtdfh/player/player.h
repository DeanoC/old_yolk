#ifndef YOLK_VTDFH_PLAYER_H_
#define YOLK_VTDFH_PLAYER_H_ 1

#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "gui/swfruntime/player.h"

namespace Scene {
	class CylinderColShape;
	class PhysicSensor;
}

class ZarchCam;
class ObjectCam;
class BuggyCam;
class Buggy;

class Player {
public:
	Player( SceneWorldPtr _world, int _localPlayerNum, Core::TransformNode* startNode );
	~Player();
	
private:
	void update( float timeS );
	bool findHeightBelow( float& height );
	void debugCallback( void );
	void gameControls( const InputFrame& input );
	void freeControls( const InputFrame& input );
	void renderable2DCallback( Scene::RenderContext* _ctx );

	const int								localPlayerNum;
	SceneWorldPtr 							world;

	Updatable								updater;
	ThingPtr  								myThingy;

	std::shared_ptr<InputHandlerContext>	inputContext;
	std::shared_ptr<ZarchCam>				zarchCam;
	std::shared_ptr<BuggyCam>				buggyCam;
	bool									freeControl; // debug free/god movement

	Math::Matrix4x4							transformMatrix;
	Core::TransformNode						playerTransform;

	std::shared_ptr<Scene::CylinderColShape>	rangedColShape;
	std::shared_ptr<Scene::CylinderColShape>	bodyColShape;
	std::shared_ptr<Scene::PhysicSensor>		rangedSensor;
	std::shared_ptr<Scene::PhysicSensor>		bodySensor;

	Core::ScopedResourceHandle<Swf::PlayerHandle> 		flashTest;

	std::shared_ptr<Buggy>						buggy;
};

#endif