#ifndef _YOLK_LOCALWORLD_SCENE_WORLD_H_
#define _YOLK_LOCALWORLD_SCENE_WORLD_H_ 1

#include "scene/renderworld.h"
#include "scene/physicsworld.h"
#include "thing.h"
#include "updatable.h"

namespace Scene {
	class Camera;
}
class EnemyThingComponent;

class SceneWorld : public Scene::RenderWorld {
public:
	SceneWorld();
	~SceneWorld() { reset(); }

	void reset();
	void add( ThingPtr _thing );
	void remove( ThingPtr _thing );

	void update( float delta );

	Scene::PhysicsWorld& getPhysicsWorld() const { return *physicsWorld.get(); }

protected:
	typedef tbb::concurrent_vector< ThingPtr > 				ThingContainer;
	typedef tbb::concurrent_vector< Scene::PhysicalPtr > 	PhysicsContainer;

	// container shortcuts (save iterating and getting component each frame
	typedef tbb::concurrent_vector< Updatable* > 			UpdatableContainer; // lifetime of object must match its matching thing
	typedef tbb::concurrent_vector< EnemyThingComponent* >	EnemyContainer; // lifetime of object must match its matching thing

	std::unique_ptr<Scene::PhysicsWorld>		physicsWorld;

	ThingContainer								things;

	PhysicsContainer							staticPhysicals;
	PhysicsContainer							dynamicPhysicals;

	UpdatableContainer							updatables;
	EnemyContainer								enemies;
};

typedef std::shared_ptr<SceneWorld> SceneWorldPtr;

#endif