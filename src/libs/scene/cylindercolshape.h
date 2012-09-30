#pragma once
#ifndef YOLK_SCENE_CYLINDERCOLSHAPE_H_
#define YOLK_SCENE_CYLINDERCOLSHAPE_H_ 1

#include "collisionshape.h"

namespace Core {
	class AABB;
}
class BinProperties;
class btCollisionShape;
class btCylinderShape;
namespace Scene {
	class CylinderColShape : public CollisionShape {
	public:
		CylinderColShape( const Core::AABB& extents, int axis = 1 /* X = 0, Y = 1, Z = 2 */ );
		virtual ~CylinderColShape();	

		virtual btCollisionShape* getBTCollisionShape() const override;
	protected:
		btCylinderShape*				cylinderShape;
	};
}

#endif
