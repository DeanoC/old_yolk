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

// an index (as a void*) to this is stored in the bullet collision object user pointer
// allows one to get the thing etc. that was involved in the collision/sense
struct SceneBulletHandle {
	friend class SceneWorld;
	ThingPtr	thingy;	// The thingy that owned the bullet collision object
	bool		sensor;	// whether its a ghost sensor or a rigidbody
	int			index;	// which physical caused it
private:
	SceneBulletHandle( const ThingPtr& _thingy, bool _sensor, int _index ) :
		thingy(_thingy), sensor(_sensor), index(_index ) {}
};

class SceneWorld : public Scene::RenderWorld {
public:
	SceneWorld();
	~SceneWorld() { reset(); }

	void reset();
	void add( ThingPtr _thing );
	void remove( ThingPtr _thing );

	void update( float delta );

	Scene::PhysicsWorld& getPhysicsWorld() const { return *physicsWorld.get(); }

	const SceneBulletHandle& convertBulletUserPointer( void* userPointer ) {
		return bulletHandles[ (size_t)userPointer ];
	}

protected:
	typedef tbb::concurrent_vector< ThingPtr > 				ThingContainer;
	typedef tbb::concurrent_vector< Scene::PhysicalPtr > 	PhysicsContainer;
	typedef tbb::concurrent_vector< SceneBulletHandle > 	BulletHandleContainer;

	// container shortcuts (save iterating and getting component each frame
	typedef tbb::concurrent_vector< Updatable* > 			UpdatableContainer; // lifetime of object must match its matching thing
	typedef tbb::concurrent_vector< EnemyThingComponent* >	EnemyContainer; // lifetime of object must match its matching thing

	std::unique_ptr<Scene::PhysicsWorld>		physicsWorld;

	ThingContainer								things;

	PhysicsContainer							staticPhysicals;
	PhysicsContainer							dynamicPhysicals;
	BulletHandleContainer						bulletHandles;

	UpdatableContainer							updatables;
	EnemyContainer								enemies;
};

typedef std::shared_ptr<SceneWorld> SceneWorldPtr;

#endif