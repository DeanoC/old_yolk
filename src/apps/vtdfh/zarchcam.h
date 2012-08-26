#ifndef VOLK_VTDFH_ZARCH_CAM_H_
#define VOLK_VTDFH_ZARCH_CAM_H_ 1

#include "scene/camera.h"
#include "localworld/thing.h"
#include "localworld/updatable.h"

class ZarchCam : public Scene::Camera, public Updatable {
public:
	ZarchCam();

	void setTrackingThing( ThingPtr thing );
	void setOffset( const Math::Vector3& _offset ) { offset = _offset; }

	void update( float deltaS ) override;
protected:
	ThingPtr				thingy;
	Math::Vector3			offset;
};

#endif