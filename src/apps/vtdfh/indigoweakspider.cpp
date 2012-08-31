#include "vtdfh.h"
#include "core/debug_render.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "zarchcam.h"
#include "objectcam.h"
#include "indigoweakspider.h"

IndigoWeakSpider::IndigoWeakSpider( SceneWorldPtr _world, Core::TransformNode* startNode ) :
	world( _world ) {

	namespace arg = std::placeholders;

	spider.reset(  ThingFactory::createThingFromHier( std::make_shared<Scene::Hier>( "spider" ) ) );
	updater.updateCallback = std::bind( & IndigoWeakSpider::update, this, arg::_1 );
	spider->addComponent( &updater );

	world->add( spider ); // render object
	spider->getRenderable()->getTransformNode()->setLocalPosition( startNode->getLocalPosition() );
	spider->getRenderable()->getTransformNode()->setLocalOrientation( startNode->getLocalOrientation() );
	if( spider->getPhysicalCount() ) {
		spider->getPhysical()->getRigidBody()->setAngularFactor( btVector3(0.0f, 1.0f,0.0f) );
		spider->getPhysical()->syncBulletTransform();
	}
}


IndigoWeakSpider::~IndigoWeakSpider() {
	world->remove( spider );
}

void IndigoWeakSpider::update( float timeMS ) {
	if( !spider->getPhysicalCount() ) return;

	auto p = spider->getPhysical();
	auto r = spider->getRenderable();
	auto rb = p->getRigidBody();

	btVector3 colCenter;
	float colRadius;
	rb->getCollisionShape()->getBoundingSphere( colCenter, colRadius );

	// anti-gravity
	btVector3 force(0,(9.81f*spider->getPhysical()->getMass() ),0);
	spider->getPhysical()->getRigidBody()->applyCentralImpulse( force * timeMS );
}