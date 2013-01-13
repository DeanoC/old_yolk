#ifndef VOLK_VTDFH_TRACKING_CAM_H_
#define VOLK_VTDFH_TRACKING_CAM_H_ 1

#include "scene/camera.h"
#include "localworld/thing.h"
#include "localworld/updatable.h"

class TrackingCam : public Scene::Camera {
public:
	TrackingCam();

	void setTrackingThing( ThingPtr thing );

protected:
	void update( float deltaS );
	ThingPtr		thingy;
};

#endif