#include "bowlball.h"
#include "localworld/thing.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "buggycam.h"

BuggyCam::BuggyCam() {
	// TODO proper projection settings
	setProjection( 90.0f * Math::degree_to_radian<float>(), 10.f/16.f, 0.1f, 250.0f );
	travelDirection = Math::Vector3( 0, 0, 1 );
}

void BuggyCam::update( float deltaS ) {
	if( buggy == nullptr ) {
		viewMatrix = Math::IdentityMatrix();
		return;
	}
	const auto& tr = buggy->buggyTransform;
	const Math::Matrix4x4 matrix = tr.getWorldMatrix();
	const Math::Vector3 lclOffset = Math::Transform( offset, matrix );

	const Math::Vector3 eye( lclOffset );
	const Math::Vector3 target = eye + Math::Normalise( Math::Vector3( buggy->vehicle->getForwardVector() ) );

	setView( Math::Matrix4x4( Math::CreateLookAtMatrix( eye, target, Math::GetYAxis( matrix ) ) ) );
	
}
