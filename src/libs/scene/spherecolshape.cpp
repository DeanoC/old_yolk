#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "spherecolshape.h"

namespace Scene {
	SphereColShape::SphereColShape( float _radius ) :
		sphereShape( nullptr ) 
	{
		sphereShape  = CORE_NEW btSphereShape( _radius );
	}
	SphereColShape::~SphereColShape() {
		CORE_DELETE sphereShape;
	}
	btCollisionShape* SphereColShape::getBTCollisionShape() const { 
		return static_cast<btCollisionShape*>(sphereShape); 
	}

}