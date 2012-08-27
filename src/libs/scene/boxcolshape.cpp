#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "core/aabb.h"
#include "boxcolshape.h"

namespace Scene {
	BoxColShape::BoxColShape( const Core::AABB& aabb ) :
		boxShape( nullptr ) 
	{
		const auto& v = aabb.getHalfLength();
		boxShape  = CORE_NEW btBoxShape( btVector3(v[0], v[1], v[2]) );
	}
	BoxColShape::~BoxColShape() {
		CORE_DELETE boxShape;
	}
	btCollisionShape* BoxColShape::getBTCollisionShape() const { 
		return static_cast<btCollisionShape*>(boxShape); 
	}

}