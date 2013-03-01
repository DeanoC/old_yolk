#include "pch.h"
#include "localworld/thing.h"
#include "zarchcam.h"

ZarchCam::ZarchCam() {
	// TODO proper projection settings
	setProjection( 70.0f * Math::degree_to_radian<float>(), 4.0/3.0, 0.1f, 200.0f );
}

void ZarchCam::update( float deltaS ) {
	if( thingy == nullptr ) {
		viewMatrix = Math::IdentityMatrix();
		return;
	}

	// TODO best fit all nodes etc.
	// TODO up, fixed axis etc.
	const auto& tr = thingy->getTransform();
	const Math::Matrix4x4 matrix = tr->getWorldMatrix();
	const Math::Vector3 target = Math::GetTranslation( matrix );
	const Math::Vector3 eye( target + offset );
	setView( Math::Matrix4x4( Math::CreateLookAtMatrix( eye, target, Math::Vector3(0,1,0) ) ) );

}

void ZarchCam::setTrackingThing( ThingPtr thing ) {
	thingy = thing;
}