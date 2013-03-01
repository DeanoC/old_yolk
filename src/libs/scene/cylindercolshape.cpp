#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "core/aabb.h"
#include "cylindercolshape.h"

namespace Scene {
	CylinderColShape::CylinderColShape( const Core::AABB& aabb, int axis ) :
		cylinderShape( nullptr ) 
	{
		const auto& v = aabb.getHalfLength();
		// use new as btCylinderShape has class specific new
		switch( axis ) {
			case 0:	cylinderShape  = new btCylinderShapeX( btVector3(v[0], v[1], v[2]) ); break;
			case 1:	cylinderShape  = new btCylinderShape( btVector3(v[0], v[1], v[2]) ); break;
			case 2:	cylinderShape  = new btCylinderShapeZ( btVector3(v[0], v[1], v[2]) ); break;
			default: CORE_ASSERT( false );
		}
	}
	CylinderColShape::~CylinderColShape() {
		CORE_DELETE cylinderShape;
	}
	btCollisionShape* CylinderColShape::getBTCollisionShape() const { 
		return static_cast<btCollisionShape*>(cylinderShape); 
	}

}