
#ifndef YOLK_LOCALWORLD_THING_H_
#define YOLK_LOCALWORLD_THING_H_ 1

#include "thingid.h"
#include "scene/physical.h"
#include "scene/physicsensor.h"
#include "scene/renderable.h"
#include "scene/hier.h"
#include "localworld/thingcomponent.h"


enum THING_BROAD_CATEGORIES {
	TBC_NONE	= 0,
	// bullet uses some, these 3 map to btBroadPhaseProxy::CollisionFilterGroup, first 3 can different via isXXX on btCollisionObject
	TBC_DEBRIS		= BIT(3),
	TBC_SENSOR		= BIT(4),
	TBC_CHARACTER	= BIT(5),

	// game bits
	TBC_WORLD		= BIT(6),
	TBC_PLAYER		= BIT(7),
	TBC_ENEMY		= BIT(8),
	TBC_ITEM		= BIT(9),

	// when used as a mask, this is everything
	TBC_ALL			= ~0
};

class Thing {
public:
	friend class ThingFactory;
	~Thing();

	int getPhysicalCount() const { return preps.size(); }
	int getRenderableCount() const { return vreps.size(); }
	int getPhysicSensorCount() const { return sreps.size(); }

	Scene::RenderablePtr getRenderable( int nodeId = 0 ) const {
		CORE_ASSERT( nodeId >= 0 && nodeId < getRenderableCount() );
		return vreps[nodeId];		
	}

	Scene::PhysicalPtr getPhysical( int nodeId ) const {
		CORE_ASSERT( nodeId >= 0 && nodeId < getPhysicalCount() );
		return preps[nodeId];		
	}
	Scene::PhysicSensorPtr getPhysicSensor( int nodeId ) const {
		CORE_ASSERT( nodeId >= 0 && nodeId < getPhysicSensorCount() );
		return sreps[nodeId];		
	}
	uint16_t getPhysicalCollisionMask( int nodeId ) const {
		CORE_ASSERT( nodeId >= 0 && nodeId < getPhysicalCount() );
		return prepColMasks[nodeId];		
	}
	uint16_t getPhysicSensorCollisionMask( int nodeId  ) const {
		CORE_ASSERT( nodeId >= 0 && nodeId < getPhysicSensorCount() );
		return srepColMasks[nodeId];		
	}

	Core::TransformNode* getTransform() const {
		if( vreps.size() > 0 ) {
			return vreps[0]->getTransformNode();
		} else if( preps.size() > 0 ) {
			return preps[0]->getTransformNode();
		} else if( sreps.size() > 0 ) {
			return sreps[0]->getTransformNode();
		} else {
			return nullptr;
		}
	}
	const ThingId getId() const { return id; }

	size_t add( Scene::RenderablePtr vrep );
	size_t add( Scene::PhysicalPtr prep, uint32_t _collisionMask );
	size_t add( Scene::PhysicSensorPtr srep, uint32_t _collisionMask );

	uint16_t getBroadCategories() const { return broadCategories; }

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
	explicit Thing( uint32_t _bc, const ThingId _id ) : broadCategories( _bc ), id( _id ) {}

	typedef std::unordered_map< uint32_t, ThingComponent* > ComponentMap;

	ComponentMap components;

	const ThingId						id;
	const uint16_t						broadCategories;

	std::vector<Scene::PhysicalPtr>		preps;
	std::vector<uint16_t>				prepColMasks;
	std::vector<Scene::RenderablePtr>	vreps;
	std::vector<Scene::PhysicSensorPtr>	sreps;
	std::vector<uint16_t>				srepColMasks;
};

typedef std::shared_ptr<Thing> ThingPtr;

#endif