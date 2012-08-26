#ifndef _YOLK_LOCALWORLD_SCENE_WORLD_H_
#define _YOLK_LOCALWORLD_SCENE_WORLD_H_ 1

#include "scene/renderworld.h"
#include "scene/physicsworld.h"
#include "thing.h"
#include "updatable.h"

namespace Scene {
	class Camera;
}

class SceneWorld : public Scene::RenderWorld {
public:
	SceneWorld();

	void reset();
	void add( ThingPtr _thing );
	void remove( ThingPtr _thing );
	void add( Updatable* _updatable );
	void remove( Updatable* _updatable );

	void update( float delta );

	Scene::PhysicsWorld& getPhysicsWorld() const { return *physicsWorld.get(); }

protected:
	typedef tbb::concurrent_vector< ThingPtr > 				ThingContainer;
	typedef tbb::concurrent_vector< Scene::PhysicalPtr > 	PhysicsContainer;
	typedef tbb::concurrent_vector< Updatable* > 			UpdatableContainer; // lifetime of object must match its matching thing

	std::unique_ptr<Scene::PhysicsWorld>		physicsWorld;
	ThingContainer								things;
	UpdatableContainer							updatables;
	PhysicsContainer							staticPhysicals;
	PhysicsContainer							dynamicPhysicals;
};

typedef std::shared_ptr<SceneWorld> SceneWorldPtr;

#endif