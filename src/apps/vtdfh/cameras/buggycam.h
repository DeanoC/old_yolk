#ifndef VOLK_VTDFH_BUGGY_CAM_H_
#define VOLK_VTDFH_BUGGY_CAM_H_ 1

#include "scene/camera.h"
#include "player/buggy.h"

class BuggyCam : public Scene::Camera {
public:
	BuggyCam();

	void setBuggy( std::shared_ptr<Buggy> _buggy ) { buggy = _buggy; }
	void setOffset( const Math::Vector3& _offset ) { offset = _offset; }

	void update( float deltaS );
protected:
	std::shared_ptr<Buggy>		buggy;
	Math::Vector3				offset;

	Math::Vector3				travelDirection;
};

#endif