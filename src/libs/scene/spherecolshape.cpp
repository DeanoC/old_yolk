#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "spherecolshape.h"

namespace Scene {
	SphereColShape::SphereColShape( float _radius ) :
		sphereShape( nullptr ) {
		// use new as btSphereShape as class specific new
		sphereShape  = new btSphereShape( _radius );
	}
	SphereColShape::~SphereColShape() {
		// use new as btSphereShape as class specific new
		CORE_DELETE sphereShape;
	}
	btCollisionShape* SphereColShape::getBTCollisionShape() const { 
		return static_cast<btCollisionShape*>(sphereShape); 
	}

}