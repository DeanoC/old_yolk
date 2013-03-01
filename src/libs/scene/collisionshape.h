#pragma once
#ifndef YOLK_SCENE_COLLISION_SHAPE_H_
#define YOLK_SCENE_COLLISION_SHAPE_H_ 1

class btCollisionShape;

namespace Scene {

	class CollisionShape {
	public:
		virtual ~CollisionShape(){};

		virtual btCollisionShape* getBTCollisionShape() const = 0;

	protected:
		CollisionShape(){};
	};
}
#endif
