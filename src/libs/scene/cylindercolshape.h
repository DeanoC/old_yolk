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
		CylinderColShape( const Core::AABB& extents );
		virtual ~CylinderColShape();	

		virtual btCollisionShape* getBTCollisionShape() const override;
	protected:
		btCylinderShape*				cylinderShape;
	};
}

#endif
