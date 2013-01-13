#ifndef YOLK_VTDFH_BUGGY_H_
#define YOLK_VTDFH_BUGGY_H_ 1

#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"

namespace Scene {
	class CollisionShape;
	class CompoundColShape;
	class CylinderColShape;
	class DynamicPhysical;
}
class btDefaultVehicleRaycaster;
class btRaycastVehicle;

class Buggy {
public:
	friend class Player;
	friend class BuggyCam;
	Buggy( SceneWorldPtr _world, Core::TransformNode* _startNode );
	~Buggy();

	// -1 to 1 (hard left to hard right )
	void turn( float force );
	// 0 to 1, nothing on the accelerator to too the metal
	void accelerate( float force );
	// 0 to 1 nothing on the break to hard down (double k cos break is a keyword)
	void breakk( float force );

protected:
	void update( float timeInSeconds );

	SceneWorldPtr 							world;
	Updatable								updater;
	ThingPtr  								myThingy;

	Math::Matrix4x4							transformMatrix;
	Core::TransformNode						buggyTransform;

	btRaycastVehicle::btVehicleTuning				tuning;
	std::shared_ptr<Scene::CompoundColShape>		compoundShape;
	std::shared_ptr<Scene::CylinderColShape>		wheelShape;
	std::shared_ptr<Scene::DynamicPhysical>			carChassis;
	btDefaultVehicleRaycaster*						vehicleRayCaster;
	btRaycastVehicle*								vehicle;

};

#endif