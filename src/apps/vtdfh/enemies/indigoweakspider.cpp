#include "vtdfh.h"
#include "core/debug_render.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "enemies/indigoweakspider.h"

IndigoWeakSpider::IndigoWeakSpider( SceneWorldPtr _world, Core::TransformNode* startNode ) :
	world( _world ) {

	namespace arg = std::placeholders;

	spider.reset(  ThingFactory::createThingFromHier( std::make_shared<Scene::Hier>( "spider" ), TBC_ENEMY ) );
	updater.updateCallback = std::bind( & IndigoWeakSpider::update, this, arg::_1 );
	spider->addComponent( &updater );

	world->add( spider ); // render object
	spider->getRenderable()->getTransformNode()->setLocalPosition( startNode->getLocalPosition() );
	spider->getRenderable()->getTransformNode()->setLocalOrientation( startNode->getLocalOrientation() );
	if( spider->getPhysicalCount() ) {
		spider->getPhysical(0)->getRigidBody()->setAngularFactor( btVector3(0.0f, 1.0f,0.0f) );
		spider->getPhysical(0)->syncBulletTransform();
	}
}


IndigoWeakSpider::~IndigoWeakSpider() {
	world->remove( spider );
}

void IndigoWeakSpider::update( float timeMS ) {
	if( !spider->getPhysicalCount() ) return;

	auto p = spider->getPhysical(0);
	auto r = spider->getRenderable();
	auto rb = p->getRigidBody();

	btVector3 colCenter;
	float colRadius;
	rb->getCollisionShape()->getBoundingSphere( colCenter, colRadius );

	// anti-gravity
	btVector3 force(0,(9.81f*p->getMass() ),0);
	rb->applyCentralImpulse( force * timeMS );
}