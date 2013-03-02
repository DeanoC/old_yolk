#include "pch.h"
#include <boost/lexical_cast.hpp>
#include "core/debug_render.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "scene/cylindercolshape.h"
#include "scene/physicsensor.h"
#include "cameras/zarchcam.h"
#include "gui/swfruntime/movieclip.h"
#include "player.h"

Player::Player( SceneWorldPtr _world, int _localPlayerNum, Core::TransformNode* _startNode ) :
	world( _world ),
	localPlayerNum( _localPlayerNum ), 
	playerTransform( transformMatrix ),
	freeControl( false ) {

	namespace arg = std::placeholders;

	basicUI.reset( Scene::TextureAtlasHandle::load( "basic_ui" ) );

	flashTest.reset( Swf::PlayerHandle::load( "testdialog" ) );
	auto fl = flashTest.acquire();
	auto r2d = std::bind( &Player::renderable2DCallback, this, arg::_1, arg::_2 );
	world->addRenderable2D( std::make_shared<std::function< void (const Scene::ScreenPtr, Scene::RenderContext*)>>(r2d) );

	myThingy.reset( ThingFactory::createThingFromHier( std::make_shared<Scene::Hier>( "drtom" ), TBC_PLAYER )  );

	Core::AABB aabb( Math::Vector3( -10, 0, -10 ), Math::Vector3( 10, 2, 10 ) );
	rangedColShape = std::make_shared<Scene::CylinderColShape>( aabb );
	rangedSensor = std::make_shared<Scene::PhysicSensor>( &playerTransform, rangedColShape );
	myThingy->add( rangedSensor, TBC_ENEMY | TBC_ITEM );

	aabb = Core::AABB( Math::Vector3( -1, 0, -1 ), Math::Vector3( 1, 2, 1 ) );
	bodyColShape = std::make_shared<Scene::CylinderColShape>( aabb );
	bodySensor = std::make_shared<Scene::PhysicSensor>( &playerTransform, bodyColShape );
	myThingy->add( bodySensor, TBC_WORLD | TBC_ENEMY | TBC_ITEM );

	updater.updateCallback = std::bind( &Player::update, this, arg::_1 );
	myThingy->addComponent( &updater );

	zarchCam = std::make_shared<ZarchCam>();
	zarchCam->setTrackingThing( myThingy );
	zarchCam->setOffset( Math::Vector3( 4.0, 6.0, -4.0 ) );

	inputContext = std::dynamic_pointer_cast<InputHandlerContext>( Core::DevelopmentContext::getr().getContext( "InputHandler" ) );
	inputContext->setCamera( zarchCam );

	Core::DevelopmentContext::get()->activateContext( "InputHandler" );
	
	if( _startNode ) {
		myThingy->getTransform()->setLocalPosition( _startNode->getLocalPosition() );
		myThingy->getTransform()->setLocalOrientation( _startNode->getLocalOrientation() );
	}

	for( int i = 0; i < myThingy->getPhysicSensorCount(); ++i ) {
		myThingy->getPhysicSensor(i)->syncBulletTransform();
	}
	for( int i = 0; i < myThingy->getPhysicalCount(); ++i ) {
		myThingy->getPhysical(i)->syncBulletTransform();
	}

	world->debugRenderCallback = std::bind( &Player::debugCallback, this );
	world->add( myThingy );
}


Player::~Player() {
	zarchCam.reset();
	world->remove( myThingy );

	myThingy.reset();
	rangedSensor.reset();
	rangedColShape.reset();
	bodySensor.reset();
	bodyColShape.reset();
	flashTest.reset();
}

bool Player::findHeightBelow( float& height ) {
	// find height below via area's bullet raycast
	auto dw = world->getPhysicsWorld().getDynamicsWorld();
	btTransform tr;
	const Math::Matrix4x4& matrix = myThingy->getTransform()->getWorldMatrix();
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

void Player::update( float timeInSeconds ) {
	InputFrame input;

	auto fl = flashTest.acquire();
	fl->advance( timeInSeconds * 1000.0f );

	while( inputContext->dequeueInputFrame( &input ) ) {
		if( input.pad[0].debugButton1 ) {
			auto& pw = world->getPhysicsWorld();
			pw.nextPhysicsDebugMode();
		}
		if( input.pad[0].debugButton2 ) {
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
			
			for( int i = 0; i < myThingy->getPhysicalCount(); ++i ) {
				myThingy->getPhysical(i)->syncBulletTransform();
			}
		}
	}
	// manually drive camera updates TODO add back to updatables/things list??
	zarchCam->update( timeInSeconds );
}

void Player::freeControls( const InputFrame& input ) {
	auto transform = myThingy->getTransform();
	Math::Quaternion rot = transform->getLocalOrientation();
	Math::Matrix4x4 rm = Math::CreateRotationMatrix( rot );
	Math::Vector3 yvec = Math::GetYAxis( rm );

	if( fabsf(input.pad[0].YAxisMovement) > 1e-5f) {
		Math::Vector3 zvec = Math::GetZAxis( rm );
		Math::Vector3 fv = (zvec * input.pad[0].YAxisMovement  * input.deltaTime) * 10.f;
		myThingy->getTransform()->setLocalPosition( transform->getLocalPosition() + fv );
	}

	if( fabsf(input.pad[0].XAxisMovement) > 1e-5f) {
		Math::Vector3 xvec = Math::GetXAxis( rm );
		Math::Vector3 fv = (xvec * input.pad[0].XAxisMovement  * input.deltaTime) * 10.f;
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
}

void Player::gameControls( const InputFrame& input ) {
	
	auto transform = myThingy->getTransform();
	Math::Vector3 xvec = Math::Vector3(1,0,0);
	Math::Vector3 zvec = Math::Vector3(0,0,-1);

	if( fabsf(input.pad[0].XAxisMovement) > 1e-5f) {
		Math::Vector3 fv = (zvec * input.pad[0].XAxisMovement  * input.deltaTime) * 10.f;
		myThingy->getTransform()->setLocalPosition( transform->getLocalPosition() + fv );
	}

	if( fabsf(input.pad[0].YAxisMovement) > 1e-5f) {
		Math::Vector3 fv = (xvec * input.pad[0].YAxisMovement  * input.deltaTime) * 10.f;
		myThingy->getTransform()->setLocalPosition( transform->getLocalPosition() + fv );
	}

	mousePos.x = input.absoluteMouseX;
	mousePos.y = input.absoluteMouseY;
}

void Player::debugCallback( void ) {

//	objectCam->getFrustum().debugDraw( Core::RGBAColour(1,0,0,1) );
	
/*	Math::Matrix4x4 rm( myThingy->getTransform()->getWorldMatrix() );
	Math::Vector3 xvec = Math::GetXAxis( rm );
	Math::Vector3 yvec = Math::GetYAxis( rm );
	Math::Vector3 zvec = Math::GetZAxis( rm );
	Math::Vector3 pos = Math::GetTranslation( rm );
	Core::g_pDebugRender->worldLine( Core::RGBAColour(1,1,0,0), pos, pos + zvec * 10 );
*/

	for( int i = 0; i < myThingy->getPhysicSensorCount(); ++i ) {
		auto& sensor = myThingy->getPhysicSensor(i);
		for( auto it : *sensor ) {
			Math::Matrix4x4 sm = sensor->getTransformNode()->getWorldMatrix();
			const auto& physDesc = world->convertBulletUserPointer( it->getUserPointer() );

			const auto& rigid = btRigidBody::upcast( it );
			if( !(rigid->getBroadphaseProxy()->m_collisionFilterGroup & TBC_WORLD) ) {
				btVector3 minAABB, maxAABB;
				rigid->getAabb( minAABB, maxAABB );
//				Core::g_pDebugRender->worldLine( Core::RGBAColour(i/2.f,i/4.f,0,0), Math::GetTranslation(sm), 
//						Math::Vector3((minAABB[0]+maxAABB[0])/2.f, maxAABB[1], (minAABB[2]+maxAABB[2])/2.0f ) );
			}
		}
	}

}

void Player::renderable2DCallback( const Scene::ScreenPtr _screen, Scene::RenderContext* _ctx ) {
	auto fl = flashTest.tryAcquire();
	if( fl ) {
//		fl->getRoot()->setMouseInput( mousePos.x,mousePos.y input.mouseLeftButton, input.mouseRightButton );
//		fl->getRoot()->setProperty( "A", CORE_GC_NEW Swf::AsObjectString( boost::lexical_cast<std::string>( (int)buggy->vehicle->getCurrentSpeedKmHour() ) + "km/h" ) );
		fl->getRoot()->setProperty( "Col1Row1", CORE_GC_NEW Swf::AsObjectString( "test" ) );
		fl->getRoot()->setProperty( "Col1Row2", CORE_GC_NEW Swf::AsObjectString( "Dirt" ) );
		fl->getRoot()->setProperty( "Col1Row3", CORE_GC_NEW Swf::AsObjectString( "Embegin" ) );
		fl->display( _ctx );
	}

	// display mouse pointer
	_screen->getComposer()->putSprite( basicUI.get(), 1, 
						Scene::ImageComposer::ALPHA_BLEND, 
						Math::Vector2( _screen->getNDCX( mousePos.x ), _screen->getNDCY( mousePos.y ) ),
						Math::Vector2( 0.015f, 0.02f ),
						Core::RGBAColour::unpackARGB(0xFFFFFFFF),
						0 );
}
