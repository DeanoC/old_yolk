#ifndef VOLK_VTDFH_OBJECT_CAM_H_
#define VOLK_VTDFH_OBJECT_CAM_H_ 1

#include "scene/camera.h"
#include "localworld/thing.h"

class ObjectCam : public Scene::Camera {
public:
	ObjectCam();

	void setObject( ThingPtr thing );

	void update( float deltaS );
protected:
	ThingPtr		thingy;
};

#endif