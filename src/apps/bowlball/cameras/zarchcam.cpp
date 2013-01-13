#include "bowlball.h"
#include "localworld/thing.h"
#include "zarchcam.h"

ZarchCam::ZarchCam() {
	// TODO proper projection settings
	setProjection( 90.0f * Math::degree_to_radian<float>(), 1.0, 1.0f, 200000.0f );
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
	setView( Math::Matrix4x4( Math::CreateLookAtMatrix( eye, target, Math::Vector3(1,0,0) ) ) );

}

void ZarchCam::setTrackingThing( ThingPtr thing ) {
	thingy = thing;
}