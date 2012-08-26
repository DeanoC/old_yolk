#ifndef VOLK_VTDFH_OBJECT_CAM_H_
#define VOLK_VTDFH_OBJECT_CAM_H_ 1

#include "scene/camera.h"
#include "localworld/thing.h"
#include "localworld/updatable.h"

class ObjectCam : public Scene::Camera, public Updatable {
public:
	ObjectCam();

	void setObject( ThingPtr thing );

	void update( float deltaS ) override;
protected:
	ThingPtr		thingy;
};

#endif