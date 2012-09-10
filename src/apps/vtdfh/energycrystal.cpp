#include "vtdfh.h"
#include "core/debug_render.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "energycrystal.h"

EnergyCrystal::EnergyCrystal( SceneWorldPtr _world, Core::TransformNode* startNode ) :
	world( _world ) {

	namespace arg = std::placeholders;

	crystal.reset(  ThingFactory::createThingFromHier( std::make_shared<Scene::Hier>( "energycrystal" ) ) );

	world->add( crystal ); // render object
	crystal->getRenderable()->getTransformNode()->setLocalPosition( startNode->getLocalPosition() );
	crystal->getRenderable()->getTransformNode()->setLocalOrientation( startNode->getLocalOrientation() );
	if( crystal->getPhysicalCount() ) {
		crystal->getPhysical()->syncBulletTransform();
	}
}

EnergyCrystal::~EnergyCrystal() {
	world->remove( crystal );
}