//!-----------------------------------------------------
//!
//! \file sceneworld.cpp
//!
//!-----------------------------------------------------
#include "localworld.h"
#include "scene/physicsworld.h"
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
	for( int i = 0;i < _thing->getRenderableCount(); ++i ) {
		addRenderable( _thing->getRenderable( i ) );
	}
	for( int i = 0;i < _thing->getPhysicalCount(); ++i ) {
		auto physical = _thing->getPhysical(i);
		if( physical->getMass() <= 0 ) {
			staticPhysicals.push_back( physical );
		} else {
			dynamicPhysicals.push_back( physical );
		}
		physicsWorld->addPhysical( physical );
	}
	things.push_back( _thing );
}

void SceneWorld::add( Updatable* _updatable ) {
	updatables.push_back( _updatable );
}

void SceneWorld::remove( Updatable* _updatable ) {
	auto u = std::find( updatables.begin(), updatables.end(), _updatable );
	CORE_ASSERT( (u != updatables.end()) && "updatable is not in this  world");
	*u = nullptr; // concurrent vector doesn't shrink until a clear
}

void SceneWorld::remove( ThingPtr _thing ) {

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
		physicsWorld->removePhysical( physical );
	}
	for( int i = 0;i < _thing->getRenderableCount(); ++i ) {
		removeRenderable( _thing->getRenderable( i ) );
	}

	auto t = std::find( things.begin(), things.end(), _thing );
	CORE_ASSERT( (t != things.end()) && "thing is not in this  world");
	*t = nullptr; // concurrent vector doesn't shrink until a clear
}

void SceneWorld::update( float delta ) {
	for( auto it : updatables ) {
		if( it ) {
			it->update( delta );
		}
	}
	physicsWorld->doSim( delta );
}
