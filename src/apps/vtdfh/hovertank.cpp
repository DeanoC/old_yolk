#include "vtdfh.h"
#include "core/debug_render.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "zarchcam.h"
#include "objectcam.h"
#include "hovertank.h"

HoverTank::HoverTank( SceneWorldPtr _world, int _localPlayerNum ) :
	world( _world ),
	speed( 1.0f * 1e7f ),		// newton second
	angularSpeed( 2.0f * 1e7f ),		// newton second
	localPlayerNum( _localPlayerNum ) {

	ship = std::make_shared<Thing>( std::make_shared<Scene::Hier>( "stinger" ) );

	zarchCam = std::make_shared<ZarchCam>();
	zarchCam->setTrackingThing( ship );
	zarchCam->setOffset( Math::Vector3(0,50,0) );
	world->add( static_cast<Updatable*>( zarchCam.get() ) );
	objectCam = std::make_shared<ObjectCam>();
	objectCam->setObject( ship );
	world->add( static_cast<Updatable*>( objectCam.get() ) );

	inputContext = std::dynamic_pointer_cast<InputHandlerContext>( Core::DevelopmentContext::getr().getContext( "InputHandler" ) );
//	inputContext->setCamera( zarchCam );
	inputContext->setCamera( objectCam );

	Core::DevelopmentContext::get()->activateContext( "InputHandler" );

	world->add( ship ); // render object
	world->add( static_cast<Updatable*>( this ) ); // updateble interface
	ship->getRenderable()->getTransformNode()->setLocalPosition( Math::Vector3(100,20,0) );
	if( ship->getPhysicalCount() ) {
		ship->getPhysical()->getRigidBody()->setAngularFactor( btVector3(0.0f, 1.0f,0.0f) );
		ship->getPhysical()->syncBulletTransform();
	}
}


HoverTank::~HoverTank() {
	world->remove(  static_cast<Updatable*>( zarchCam.get() ) );
	zarchCam.reset();

	world->remove( static_cast<Updatable*>( this ) );
	world->remove( ship );
}

bool HoverTank::findHeightBelow( float& height ) {
	// find height below via area's bullet raycast
	auto dw = world->getPhysicsWorld().getDynamicsWorld();
	btTransform tr;
	const Math::Matrix4x4& matrix = ship->getRenderable()->getTransformNode()->getWorldMatrix();
	tr.setFromOpenGLMatrix( &matrix[0] );
	btVector3 begin = tr.getOrigin() + btVector3( 0, 200, 0 );
	btVector3 end = begin + btVector3( 0, -10000, 0 );
	btCollisionWorld::ClosestRayResultCallback rayCB( begin, end );

	dw->rayTest( begin, end, rayCB );
	if( rayCB.hasHit() ) {
		height = rayCB.m_hitPointWorld.y();
		return true;
	} else {
		return false;
	}
}

void HoverTank::update( float timeMS ) {
	if( !ship->getPhysicalCount() ) return;

	auto p = ship->getPhysical();
	auto r = ship->getRenderable();
	auto rb = p->getRigidBody();

	btVector3 colCenter;
	float colRadius;
	rb->getCollisionShape()->getBoundingSphere( colCenter, colRadius );

	InputFrame input;
	// anti-gravity
	btVector3 force(0,(9.81f*ship->getPhysical()->getMass() ),0);
	ship->getPhysical()->getRigidBody()->applyCentralImpulse( force * timeMS );

	if( inputContext->dequeueInputFrame( &input ) ) {
		rb->activate();

		if( input.pad[0].debugButton1 ) {
			auto& pw = world->getPhysicsWorld();
			pw.nextPhysicsDebugMode();
		}
		if( input.pad[0].debugButton2 ) {
			if( inputContext->getCamera() == objectCam ) {
				inputContext->setCamera( zarchCam );
			} else {
				inputContext->setCamera( objectCam );
			}
		}
		Math::Matrix4x4 rm( r->getTransformNode()->getWorldMatrix() );
		Math::Vector3 xvec = Math::GetXAxis( rm );
		Math::Vector3 yvec = Math::GetYAxis( rm );
		Math::Vector3 zvec = Math::GetZAxis( rm );
		Math::Vector3 pos = Math::GetTranslation( rm );

		if( fabsf(input.pad[0].YAxisMovement) > 1e-5f) {
			Math::Vector3 fv = (zvec * input.pad[0].YAxisMovement  * input.deltaTime) * speed;
			btVector3 mainengine(fv[0],fv[1],fv[2]);
			rb->applyCentralImpulse( mainengine * timeMS );
		}
		if( fabsf(input.pad[0].XAxisMovement) > 1e-5f) {
			Math::Vector3 sv = (xvec * input.pad[0].XAxisMovement  * input.deltaTime) * speed;
			btVector3 sidethruster(sv[0], sv[1], sv[2]);
			rb->applyCentralImpulse( sidethruster * timeMS );
		}

		float mxdt = input.mouseX  * input.deltaTime;
		if( fabsf(mxdt) > 0.0001f ) {
			mxdt = Math::Clamp(mxdt, -0.001f, 0.001f );
			Math::Vector3 sv = (yvec * mxdt) * angularSpeed;
			btVector3 banker(sv[0],sv[1],sv[2]);
			rb->applyTorqueImpulse( banker * timeMS );
		}
	}
}