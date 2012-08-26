#include "vtdfh.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "zarchcam.h"
#include "objectcam.h"
#include "playership.h"

PlayerShip::PlayerShip( SceneWorldPtr _world, int _localPlayerNum ) :
	world( _world ),
	xRot( 0 ), yRot( 0 ), zRot( 0 ),
	curSideMotion( 0 ), curForwardMotion( 0 ),
	speed( 450.0f ),			// meters per second
	angularSpeed( 2160.f ),		// degrees per second
	localPlayerNum( _localPlayerNum ) {

	lastMouseX = 0;
	lastMouseY = 0;

	ship = std::make_shared<Thing>( std::make_shared<Scene::Hier>( "stinger" ) );

	zarchCam = std::make_shared<ZarchCam>();
	zarchCam->setTrackingThing( ship );
	zarchCam->setOffset( Math::Vector3(0,100,0) );
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

}


PlayerShip::~PlayerShip() {
	world->remove(  static_cast<Updatable*>( zarchCam.get() ) );
	zarchCam.reset();

	world->remove( static_cast<Updatable*>( this ) );
	world->remove( ship );
}

bool PlayerShip::findHeightBelow( float& height ) {
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

void PlayerShip::update( float timeMS ) {
	float height = 0;
	if( findHeightBelow( height ) ) {
		const auto wpos = ship->getRenderable()->getTransformNode()->getLocalPosition();
		ship->getRenderable()->getTransformNode()->setLocalPosition( Math::Vector3(wpos.x, height+5, wpos.z) );
	}
	InputFrame input;
	if( inputContext->dequeueInputFrame( &input ) ) {
		curSideMotion *= 0.1f * input.deltaTime;
		curForwardMotion *= 0.1f * input.deltaTime;

		if( fabs( input.pad[0].XAxisMovement ) > 1e-2f ) {
			curSideMotion += input.pad[0].XAxisMovement * speed * input.deltaTime;
//			yRot -= input.pad[0].XAxisMovement * angularSpeed  * input.deltaTime;
		}
		if( fabs( input.pad[0].YAxisMovement ) > 1e-2f ) {
			curForwardMotion += input.pad[0].YAxisMovement * speed * input.deltaTime;
		}

		if( fabsf( input.mouseX ) > 1e-8f ) {
		//	LOG(INFO) << " x " <<input.mouseX << "\n";
			yRot += (input.mouseX / 1.0f) * angularSpeed  * input.deltaTime;
		}
		while( yRot > 360.0f ) yRot -= 360.0f;
		while( yRot < 0.0f ) yRot += 360.0f;

		auto xrot = Math::degree_to_radian<float>() * xRot;
		auto yrot = Math::degree_to_radian<float>() * yRot;
		auto zrot = Math::degree_to_radian<float>() * zRot;

		auto xq = CreateRotationQuat( Math::Vector3(1,0,0), xrot );
		auto yq = CreateRotationQuat( Math::Vector3(0,1,0), yrot );
		auto zq = CreateRotationQuat( Math::Vector3(0,0,1), zrot );
		auto rq = xq * yq * zq;

		Math::Matrix4x4 rm( Math::CreateRotationMatrix( rq ) );
		Math::Vector3 xvec = Math::GetXAxis( rm );
		Math::Vector3 zvec = Math::GetZAxis( rm );

		// move along forward and side motion direction	
		auto position = ship->getRenderable()->getTransformNode()->getLocalPosition();
		position -= (xvec * curSideMotion);
		position += (zvec * curForwardMotion);
		ship->getRenderable()->getTransformNode()->setLocalPosition( position );
		ship->getRenderable()->getTransformNode()->setLocalOrientation( rq );
	}
}