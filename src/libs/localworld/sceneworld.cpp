//!-----------------------------------------------------
//!
//! \file sceneworld.cpp
//!
//!-----------------------------------------------------
#include "localworld.h"
#include "scene/physicsworld.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "updatable.h"
#include "enemythingcomponent.h"
#include "sceneworld.h"

SceneWorld::SceneWorld() {
	physicsWorld.reset( CORE_NEW Scene::PhysicsWorld() );
}

void SceneWorld::reset() {
	// things and there physics should have bee rmoved when this is called
	staticPhysicals.clear();
	dynamicPhysicals.clear();
	updatables.clear();
	things.clear();
}

void SceneWorld::add( ThingPtr _thing ) {
	CORE_ASSERT( (std::find( things.cbegin(), things.cend(), _thing ) == things.cend()) && "thing added to world twice");

	things.push_back( _thing );

	// add renderables belonging to thing
	for( int i = 0;i < _thing->getRenderableCount(); ++i ) {
		addRenderable( _thing->getRenderable( i ) );
	}
	// add physics
	for( int i = 0;i < _thing->getPhysicalCount(); ++i ) {
		auto physical = _thing->getPhysical(i);
		if( physical->getMass() <= 0 ) {
			staticPhysicals.push_back( physical );
		} else {
			dynamicPhysicals.push_back( physical );
		}
		physicsWorld->add( physical, _thing->getBroadCategories(), _thing->getPhysicalCollisionMask( i ) );
		
		// currently this array never shrinks :O
		auto bulletIndex = bulletHandles.push_back( SceneBulletHandle(_thing, false, i ) );
		physical->getRigidBody()->setUserPointer( (void*) std::distance( bulletHandles.begin(), bulletIndex ) );
	}

	for( int i = 0;i < _thing->getPhysicSensorCount(); ++i ) {
		physicsWorld->add( _thing->getPhysicSensor( i ), _thing->getBroadCategories() | TBC_SENSOR, _thing->getPhysicSensorCollisionMask( i ) );
		// currently this array never shrinks :O
		auto bulletIndex = bulletHandles.push_back( SceneBulletHandle(_thing, true, i ) );
		_thing->getPhysicSensor( i )->getGhost()->setUserPointer( (void*) std::distance( bulletHandles.begin(), bulletIndex ) );
	}

	ThingComponent* component;
	// add component fast lookup containers
	if( (component = _thing->getComponent( Updatable::COMPONENT_ID )) != nullptr ) {
		updatables.push_back( static_cast< Updatable* >( component ) );
	}
	if( (component = _thing->getComponent( EnemyThingComponent::COMPONENT_ID )) != nullptr ) {
		enemies.push_back ( static_cast< EnemyThingComponent* >( component ) );
	}

}

void SceneWorld::remove( ThingPtr _thing ) {
	auto t = std::find( things.begin(), things.end(), _thing );
	CORE_ASSERT( (t != things.end()) && "thing is not in this world");

	for( int i = 0;i < _thing->getPhysicSensorCount(); ++i ) {
		physicsWorld->remove( _thing->getPhysicSensor( i ) );
	}

	for( int i = 0;i < _thing->getPhysicalCount(); ++i ) {
		auto physical = _thing->getPhysical(i);
		if( physical->getMass() <= 0 ) {
			auto f = std::find( staticPhysicals.begin(), staticPhysicals.end(), physical );
			assert( (f != staticPhysicals.end()) && "Physics is not in this physics world");
			*f = nullptr; // concurrent vector doesn't shrink until a clear
		} else {
			auto f = std::find( dynamicPhysicals.begin(), dynamicPhysicals.end(), physical );
			CORE_ASSERT( (f != dynamicPhysicals.end()) && "Physics is not in this physics world");
			*f = nullptr; // concurrent vector doesn't shrink until a clear
		}
		physicsWorld->remove( physical );
	}
	for( int i = 0;i < _thing->getRenderableCount(); ++i ) {
		removeRenderable( _thing->getRenderable( i ) );
	}
	ThingComponent* component;
	// remove components fast lookup containers
	if( (component = _thing->getComponent( Updatable::COMPONENT_ID )) != nullptr ) {
		auto u = std::find( updatables.begin(), updatables.end(), static_cast< Updatable* >( component )  );
		*u = nullptr; // concurrent vector doesn't shrink until a clear
	}
	if( (component = _thing->getComponent( EnemyThingComponent::COMPONENT_ID )) != nullptr ) {
		auto u = std::find( enemies.begin(), enemies.end(), static_cast< EnemyThingComponent* >( component )  );
		*u = nullptr; // concurrent vector doesn't shrink until a clear
	}

	*t = nullptr; // concurrent vector doesn't shrink until a clear
}

void SceneWorld::update( float delta ) {
	// todo this mutux is WAY to big and heavy
	Core::unique_lock<Core::mutex> updateLock( *getUpdateMutex() );

	for( auto it : updatables ) {
		if( it && it->updateCallback ) {
			it->updateCallback( delta );
		}
	}
	physicsWorld->doSim( delta );
}
