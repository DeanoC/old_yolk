//!-----------------------------------------------------
//!
//! \file sceneworld.cpp
//!
//!-----------------------------------------------------
#include "localworld.h"
#include "scene/physicsworld.h"
#include "sceneworld.h"

SceneWorld::SceneWorld() :
	inputQueue( 3 ) {
	physicsWorld.reset( CORE_NEW Scene::PhysicsWorld() );
}

void SceneWorld::queueInputFrame( const InputFrame& frame ) {
	inputQueue.push_front( frame );
}

bool SceneWorld::dequeueInputFrame( InputFrame* frame ) {
	if( inputQueue.isNotEmpty() ) {
		inputQueue.pop_back( frame );
		return true;
	} else {
		return false;
	}
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

void SceneWorld::update( float delta ) {
	physicsWorld->doSim( delta );
}
