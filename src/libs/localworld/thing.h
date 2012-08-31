
#ifndef YOLK_LOCALWORLD_THING_H_
#define YOLK_LOCALWORLD_THING_H_ 1

#include "thingid.h"
#include "scene/physical.h"
#include "scene/renderable.h"
#include "scene/hier.h"
#include "localworld/thingcomponent.h"

class Thing {
public:
	friend class ThingFactory;
	~Thing();

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
	Core::TransformNode* getTransform() const {
		if( vreps[0] ) {
			return vreps[0]->getTransformNode();
		} else if( preps[0] ) {
			return preps[0]->getTransformNode();
		} else {
			return nullptr;
		}
	}
	const ThingId getId() const { return id; }

	// component interface 
	ThingComponent* getComponent( const ThingComponentId id ) const { 
		auto c = components.find(id);
		return c == components.end() ? nullptr : c->second;
	}
	void addComponent( ThingComponent* _component ) {
		CORE_ASSERT( components.find( _component->getComponentId() ) == components.end() );
		components[ _component->getComponentId() ] = _component;
	}
protected:
	explicit Thing( const ThingId _id ) : id( _id ) {}

	typedef std::unordered_map< uint32_t, ThingComponent* > ComponentMap;

	ComponentMap components;

	size_t addPhysical( Scene::PhysicalPtr prep );
	size_t addRenderable( Scene::RenderablePtr vrep );

	const ThingId						id;
	std::vector<Scene::PhysicalPtr>		preps;
	std::vector<Scene::RenderablePtr>	vreps;
};

typedef std::shared_ptr<Thing> ThingPtr;

#endif