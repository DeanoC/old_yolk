#pragma once
#ifndef YOLK_SCENE_COMPOUNDCOLSHAPE_H_
#define YOLK_SCENE_COMPOUNDCOLSHAPE_H_ 1

#include "collisionshape.h"
#include "core/transform_node.h"

class btCollisionShape;
class btCompoundShape;

namespace Scene {
	class CompoundColShape : public CollisionShape {
	public:
		CompoundColShape();
		virtual ~CompoundColShape();	
		virtual btCollisionShape* getBTCollisionShape() const override;

		void addChildShape( const Math::Vector3& _attachPt, std::shared_ptr<CollisionShape> _shape );

	protected:
		std::vector< std::shared_ptr<CollisionShape> >	owned;
		btCompoundShape*								compoundShape;
	};
}

#endif
