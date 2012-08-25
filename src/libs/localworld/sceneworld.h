#ifndef _YOLK_LOCALWORLD_SCENE_WORLD_H_
#define _YOLK_LOCALWORLD_SCENE_WORLD_H_ 1

#include "localworld/inputframe.h"
#include "core/boundedbuffer.h"
#include "scene/renderworld.h"
#include "scene/physicsworld.h"
#include "thing.h"

namespace Scene {
	class Camera;
}

class SceneWorld : public Scene::RenderWorld {
public:
	SceneWorld();

	void queueInputFrame( const InputFrame& frame );
	bool dequeueInputFrame( InputFrame* frame ); 	//!< return false if no input to dequeue

	void add( ThingPtr _thing );
	void remove( ThingPtr _thing );
	void update( float delta );
protected:

	std::unique_ptr<Scene::PhysicsWorld>		physicsWorld;					
	std::vector<ThingPtr>						things;
	std::vector<Scene::PhysicalPtr>				staticPhysicals;
	std::vector<Scene::PhysicalPtr>				dynamicPhysicals;
	Core::BoundedBuffer<InputFrame>				inputQueue;
};

typedef std::shared_ptr<SceneWorld> SceneWorldPtr;

#endif