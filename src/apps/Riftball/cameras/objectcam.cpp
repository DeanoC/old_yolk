#include "bowlball.h"
#include "localworld/thing.h"
#include "objectcam.h"

ObjectCam::ObjectCam() {
	// TODO proper projection settings
	setProjection( 90.0f * Math::degree_to_radian<float>(), 1.0, 1.0f, 12000.0f );

}

void ObjectCam::update( float deltaS ) {
	if( thingy == nullptr ) {
		viewMatrix = Math::IdentityMatrix();
		return;
	}
	const auto& tr = thingy->getTransform();
	viewMatrix = Math::InverseMatrix( tr->getWorldMatrix() );
}

void ObjectCam::setObject( ThingPtr thing ) {
	thingy = thing;
}