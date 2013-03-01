#include "bowlball.h"
#include "localworld/thing.h"
#include "trackingcam.h"

TrackingCam::TrackingCam() {
	// TODO proper projection settings
	setProjection( 90.0f * Math::degree_to_radian<float>(), 1.0, 1.0f, 5000.0f );
}

void TrackingCam::update( float deltaS ) {
	if( thingy == nullptr ) {
		viewMatrix = Math::IdentityMatrix();
		return;
	}

	// TODO best fit all nodes etc.
	// TODO offsets, fixed axis etc.
	// this is just an object cam at the moment
	const auto& tr = thingy->getTransform();
	viewMatrix = tr->getWorldMatrix();
}

void TrackingCam::setTrackingThing( ThingPtr thing ) {
	thingy = thing;
}