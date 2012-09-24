#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "core/aabb.h"
#include "cylindercolshape.h"

namespace Scene {
	CylinderColShape::CylinderColShape( const Core::AABB& aabb ) :
		cylinderShape( nullptr ) 
	{
		const auto& v = aabb.getHalfLength();
		// use new as btSphereShape as class specific new
		cylinderShape  = new btCylinderShape( btVector3(v[0], v[1], v[2]) );
	}
	CylinderColShape::~CylinderColShape() {
		CORE_DELETE cylinderShape;
	}
	btCollisionShape* CylinderColShape::getBTCollisionShape() const { 
		return static_cast<btCollisionShape*>(cylinderShape); 
	}

}