#pragma once
#ifndef YOLK_SCENE_BOXCOLSHAPE_H_
#define YOLK_SCENE_BOXCOLSHAPE_H_ 1

#include "collisionshape.h"

namespace Core {
	class AABB;
}
class BinProperties;
class btCollisionShape;
class btBoxShape;
namespace Scene {
	class BoxColShape : public CollisionShape {
	public:
		BoxColShape( const Core::AABB& extents );
		virtual ~BoxColShape();	

		virtual btCollisionShape* getBTCollisionShape() const override;
	protected:
		btBoxShape*							boxShape;
	};
}

#endif
