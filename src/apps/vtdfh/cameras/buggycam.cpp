#include "vtdfh.h"
#include "localworld/thing.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "buggycam.h"

BuggyCam::BuggyCam() {
	// TODO proper projection settings
	setProjection( 90.0f * Math::degree_to_radian<float>(), 1.0, 1.0f, 12000.0f );
	travelDirection = Math::Vector3( 0, 0, 1 );
}

void BuggyCam::update( float deltaS ) {
	if( buggy == nullptr ) {
		viewMatrix = Math::IdentityMatrix();
		return;
	}
	const auto& tr = buggy->buggyTransform;
	const Math::Matrix4x4 matrix = tr.getWorldMatrix();

	const Math::Vector3 eye( Math::GetTranslation( matrix ) + offset );
	const Math::Vector3 target = eye + Math::Vector3( buggy->vehicle->getForwardVector() );

	setView( Math::Matrix4x4( Math::CreateLookAtMatrix( eye, target, Math::GetYAxis( matrix ) ) ) );
	
}
