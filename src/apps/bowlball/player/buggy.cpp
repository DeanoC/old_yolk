#include "bowlball.h"
#include "core/development_context.h"
#include "localworld/inputhandlercontext.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "scene/boxcolshape.h"
#include "scene/compoundcolshape.h"
#include "scene/cylindercolshape.h"
#include "scene/dynamicphysical.h"
#include "buggy.h"

namespace {
///btRaycastVehicle is the interface for the constraint that implements the raycast vehicle
///notice that for higher-quality slow-moving vehicles, another approach might be better
///implementing explicit hinged-wheel constraints with cylinder collision, rather then raycasts
float	gEngineForce = 0.f;
float	gBreakingForce = 0.f;
float	gVehicleSteering = 0.0f;

float	maxEngineForce = 40.f;//this should be engine/velocity dependent
float	maxBreakingForce = 4.f;
float	steeringIncrement = 0.04f;
float	steeringClamp = 0.3f;
float	wheelRadius = 0.5f;
float	wheelWidth = 0.4f;
float	wheelFriction = BT_LARGE_FLOAT;
float	suspensionStiffness = 25.f;
float	suspensionDamping = 2.3f;
float	suspensionCompression = 4.4f;
float	rollInfluence = 0.3f;//1.0f;
btScalar suspensionRestLength(0.6f);

};

Buggy::Buggy( SceneWorldPtr _world, Core::TransformNode* _startNode ) :
	world( _world ),
	buggyTransform( transformMatrix ) {

	namespace arg = std::placeholders;

	myThingy.reset( ThingFactory::createEmptyThing( TBC_PLAYER ) );

	auto aabb = Core::AABB( Math::Vector3( -1.0f, -0.5f, -1.0f ), Math::Vector3( 1.0f, 0.5f, 1.0f ) );
	auto chassisShape = std::make_shared<Scene::BoxColShape>( aabb );
	compoundShape = std::make_shared<Scene::CompoundColShape>();
	//effectively shifts the center of mass with respect to the chassis
	compoundShape->addChildShape( Math::Vector3(0,1,0), chassisShape );

	static const float mass = 100.f;
	btVector3 inertiaTensor;
	compoundShape->getBTCollisionShape()->calculateLocalInertia( mass, inertiaTensor );
	carChassis = std::make_shared<Scene::DynamicPhysical>( &buggyTransform, compoundShape , mass, Math::Vector3( inertiaTensor ) );
	myThingy->add( carChassis, TBC_ENEMY | TBC_ITEM | TBC_WORLD );
	myThingy->getTransform()->setLocalPosition( _startNode->getLocalPosition() );
	myThingy->getTransform()->setLocalOrientation( _startNode->getLocalOrientation() );

	for( int i = 0; i < myThingy->getPhysicalCount(); ++i ) {
		myThingy->getPhysical(i)->syncBulletTransform();
	}

	updater.updateCallback = std::bind( &Buggy::update, this, arg::_1 );
	myThingy->addComponent( &updater );
	
	aabb = Core::AABB( Math::Vector3( -wheelWidth, -wheelRadius, -wheelRadius ), Math::Vector3( wheelWidth, wheelRadius, wheelRadius ) );
	wheelShape = std::make_shared<Scene::CylinderColShape>( aabb, 0 ); // x aligned cylinder of wheel radius with wheelWidth thickness
	
	/// create vehicle
	{
		
		vehicleRayCaster = new btDefaultVehicleRaycaster( world->getPhysicsWorld().getDynamicsWorld() );
		vehicle = new btRaycastVehicle(tuning, carChassis->getRigidBody(), vehicleRayCaster);
		
		///never deactivate the vehicle
		carChassis->getRigidBody()->setActivationState(DISABLE_DEACTIVATION);

		world->getPhysicsWorld().getDynamicsWorld()->addVehicle( vehicle );

		float connectionHeight = 1.2f;

		bool isFrontWheel=true;

		//choose coordinate system
		vehicle->setCoordinateSystem( 0, 1, 2 );
		btVector3 wheelDirectionCS0( 0, -1, 0 );	// wheel raycast direction
		btVector3 wheelAxleCS( -1, 0, 0 );			// axle axis

		btVector3 connectionPointCS0( 1-(0.3f*wheelWidth), connectionHeight, 2*1-wheelRadius );
		vehicle->addWheel( connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel );
		connectionPointCS0 = btVector3( -1+(0.3f*wheelWidth), connectionHeight, 2*1-wheelRadius );
		vehicle->addWheel( connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel );

		isFrontWheel = false;
		connectionPointCS0 = btVector3( -1+(0.3f*wheelWidth), connectionHeight, -2*1+wheelRadius );
		vehicle->addWheel( connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel );
		connectionPointCS0 = btVector3( 1-(0.3f*wheelWidth), connectionHeight, -2*1+wheelRadius );
		vehicle->addWheel( connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel );
		
		for (int i=0;i<vehicle->getNumWheels();i++)
		{
			btWheelInfo& wheel = vehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = suspensionDamping;
			wheel.m_wheelsDampingCompression = suspensionCompression;
			wheel.m_frictionSlip = wheelFriction;
			wheel.m_rollInfluence = rollInfluence;	
		}
	}
	
	world->add( myThingy );

}

Buggy::~Buggy() {
	world->remove( myThingy );
	myThingy.reset();

	CORE_DELETE vehicle;
	CORE_DELETE vehicleRayCaster;
}

void Buggy::update( float timeInSeconds ) {
	// 2 wheel steering and drive!

	vehicle->applyEngineForce( gEngineForce, 0 );
	vehicle->applyEngineForce( gEngineForce, 1 );
	vehicle->applyEngineForce( gEngineForce, 2 );
	vehicle->applyEngineForce( gEngineForce, 3 );

	vehicle->setSteeringValue( gVehicleSteering, 0 );
	vehicle->setSteeringValue( gVehicleSteering, 1 );
//	vehicle->setSteeringValue( gVehicleSteering, 2 );
//	vehicle->setSteeringValue( gVehicleSteering, 3 );

	vehicle->setBrake( gBreakingForce, 0 );
	vehicle->setBrake( gBreakingForce, 1 );
	vehicle->setBrake( gBreakingForce, 2 );
	vehicle->setBrake( gBreakingForce, 3 );

	vehicle->updateVehicle( timeInSeconds );

}

void Buggy::turn( float force ) {
	gVehicleSteering += steeringIncrement * force;
	if (gVehicleSteering < -steeringClamp)
			gVehicleSteering = -steeringClamp;
	if (gVehicleSteering > steeringClamp)
			gVehicleSteering = steeringClamp;

}
void Buggy::accelerate( float force ) {
	gEngineForce = maxEngineForce * force;
	gBreakingForce = 0;
}

void Buggy::breakk( float force ) {
	if( vehicle->getCurrentSpeedKmHour() < 2 ) {
		gBreakingForce = 0;
		gEngineForce = maxBreakingForce * -force;
	} else {
		gBreakingForce = maxBreakingForce * force;
		gEngineForce = 0;
	}
}

