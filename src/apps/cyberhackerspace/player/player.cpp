#include "cyberhackerspace.h"
#include <boost/lexical_cast.hpp>
#include "core/debug_render.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "scene/cylindercolshape.h"
#include "scene/physicsensor.h"
#include "cameras/objectcam.h"
#include "gui/swfruntime/movieclip.h"
#include "player.h"

Player::Player( SceneWorldPtr _world, int _localPlayerNum, Core::TransformNode* _startNode ) :
	world( _world ),
	localPlayerNum( _localPlayerNum ), 
	playerTransform( transformMatrix ),
	freeControl( false ) {
	namespace arg = std::placeholders;

	myThingy.reset( ThingFactory::createEmptyThing( TBC_PLAYER ) );

	updater.updateCallback = std::bind( &Player::update, this, arg::_1 );
	myThingy->addComponent( &updater );

	objectCam = std::make_shared<ObjectCam>();

	inputContext = std::dynamic_pointer_cast<InputHandlerContext>( Core::DevelopmentContext::getr().getContext( "InputHandler" ) );
	inputContext->setCamera( objectCam );

	Core::DevelopmentContext::get()->activateContext( "InputHandler" );

/*	
	myThingy->getTransform()->setLocalPosition( _startNode->getLocalPosition() );
	myThingy->getTransform()->setLocalOrientation( _startNode->getLocalOrientation() );

	for( int i = 0; i < myThingy->getPhysicSensorCount(); ++i ) {
		myThingy->getPhysicSensor(i)->syncBulletTransform();
	}
*/
	world->debugRenderCallback = std::bind( &Player::debugCallback, this );
	world->add( myThingy );

//	objectCam->setOffset( Math::Vector3(0,20,0) );
	objectCam->setObject( myThingy );
}


Player::~Player() {
	objectCam.reset();
	world->remove( myThingy );

	myThingy.reset();
}

void Player::update( float timeInSeconds ) {
	InputFrame input;

/*
	while( inputContext->dequeueInputFrame( &input ) ) {
		if( input.pad[0].debugButton1 ) {
			auto& pw = world->getPhysicsWorld();
			pw.nextPhysicsDebugMode();
		}
		if( input.pad[0].debugButton2 ) {
			if( inputContext->getCamera() == buggyCam ) {
				inputContext->setCamera( zarchCam );
			} else {
				inputContext->setCamera( buggyCam );
			}
		}
		if( input.pad[0].debugButton3 ) {
			freeControl ^= true;
		}

		if( freeControl ) {
			freeControls( input );
		} else {
			gameControls( input );

			for( int i = 0; i < myThingy->getPhysicSensorCount(); ++i ) {
				myThingy->getPhysicSensor(i)->syncBulletTransform();
			}
		}
	}
	*/
	// manually drive camera updates TODO add back to updatables/things list??
	objectCam->update( timeInSeconds );
}

void Player::freeControls( const InputFrame& input ) {
/*
	auto transform = myThingy->getTransform();
	Math::Quaternion rot = transform->getLocalOrientation();
	Math::Matrix4x4 rm = Math::CreateRotationMatrix( rot );
	Math::Vector3 yvec = Math::GetYAxis( rm );

	if( fabsf(input.pad[0].YAxisMovement) > 1e-5f) {
		Math::Vector3 zvec = Math::GetZAxis( rm );
		Math::Vector3 fv = (zvec * input.pad[0].YAxisMovement  * input.deltaTime) * 100000.f;
		myThingy->getTransform()->setLocalPosition( transform->getLocalPosition() + fv );
	}

	if( fabsf(input.pad[0].XAxisMovement) > 1e-5f) {
		Math::Vector3 xvec = Math::GetXAxis( rm );
		Math::Vector3 fv = (xvec * input.pad[0].XAxisMovement  * input.deltaTime) * 1000.f;
		myThingy->getTransform()->setLocalPosition( transform->getLocalPosition() + fv );
	}
	float mxdt = input.mouseX  * input.deltaTime;
	if( fabsf(mxdt) > 0.0001f ) {
		mxdt = Math::Clamp(mxdt, -0.05f, 0.05f );
		rot = rot * Math::CreateRotationQuat( Math::Vector3( 0, 1, 0 ), mxdt * 5.f );
		transform->setLocalOrientation( rot );
		rot = transform->getLocalOrientation();
	}
	float mydt = input.mouseY  * input.deltaTime;
	if( fabsf(mydt) > 0.0001f ) {
		mydt = Math::Clamp(mydt, -0.05f, 0.05f );
		rot = rot * Math::CreateRotationQuat( Math::Vector3( 1, 0, 0 ), mydt * 5.f );
		transform->setLocalOrientation( rot );
		rot = transform->getLocalOrientation();
	}
	*/
}

void Player::gameControls( const InputFrame& input ) {
}

void Player::debugCallback( void ) {
}

void Player::renderable2DCallback( Scene::RenderContext* _ctx ) {
/*	auto fl = flashTest.tryAcquire();
	if( fl ) {
		fl->getRoot()->setProperty( "speed", CORE_GC_NEW Swf::AsObjectString( boost::lexical_cast<std::string>( (int)buggy->vehicle->getCurrentSpeedKmHour() ) + "km/h" ) );
		fl->display( _ctx );
	}
	*/
}
