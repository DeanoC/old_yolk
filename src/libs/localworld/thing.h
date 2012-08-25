
#ifndef YOLK_LOCALWORLD_THING_H_
#define YOLK_LOCALWORLD_THING_H_ 1

#include "thingid.h"
#include "scene/physical.h"
#include "scene/renderable.h"
#include "scene/hier.h"

enum class ThingUpdateType {
	FROM_PHYSICS = 0x1,
	FROM_VISUALS = 0x2,
	FROM_USER	 = FROM_PHYSICS | FROM_VISUALS,
};

class Thing {
public:
	explicit Thing( Scene::HierPtr hier, const ThingId _id = NewThingId() );
	explicit Thing( const ThingId _id = NewThingId() ) : id( _id ) {}

	void updateTransform( int nodeId = 0, ThingUpdateType uptype = ThingUpdateType::FROM_USER );

	int getPhysicalCount() const { return preps.size(); }
	int getRenderableCount() const { return vreps.size(); }

	Scene::PhysicalPtr getPhysical( int nodeId = 0 ) const {
		CORE_ASSERT( nodeId >= 0 && nodeId < getPhysicalCount() );
		return preps[nodeId];		
	}


	Scene::RenderablePtr getRenderable( int nodeId = 0 ) const {
		CORE_ASSERT( nodeId >= 0 && nodeId < getRenderableCount() );
		return vreps[nodeId];		
	}
	const ThingId getId() const { return id; }

protected:
	void setPhysical( int nodeId, Scene::PhysicalPtr prep );
	void setRenderable( int nodeId, Scene::RenderablePtr vrep );

	const ThingId						id;
	std::vector<Scene::PhysicalPtr>		preps;
	std::vector<Scene::RenderablePtr>	vreps;
};

typedef std::shared_ptr<Thing> ThingPtr;

#endif