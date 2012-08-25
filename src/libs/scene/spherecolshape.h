#pragma once
#ifndef YOLK_SCENE_SPHERECOLSHAPE_H_
#define YOLK_SCENE_SPHERECOLSHAPE_H_ 1

#include "collisionshape.h"

class BinProperties;
class btCollisionShape;
class btSphereShape;
namespace Scene {
	class SphereColShape : public CollisionShape {
	public:
		SphereColShape( float radius );
		virtual ~SphereColShape();	

		virtual btCollisionShape* getBTCollisionShape() const override;
	protected:
		btSphereShape*							sphereShape;
	};
}

#endif
