#pragma once
#ifndef YOLK_SCENE_MESHCOLSHAPE_H_
#define YOLK_SCENE_MESHCOLSHAPE_H_ 1

#include "collisionshape.h"

class btCollisionShape;
class btTriangleIndexVertexArray;
class btBvhTriangleMeshShape;
namespace Scene {
	class MeshColShape : public CollisionShape {
	public:
		MeshColShape( std::shared_ptr<Core::BinPropertyResource> _props );
		virtual ~MeshColShape();	

		btCollisionShape* getBTCollisionShape() const override;
	protected:
		std::shared_ptr<Core::BinPropertyResource>		properties;
		btTriangleIndexVertexArray*						indexVertexArray;
		btBvhTriangleMeshShape*							trimeshShape;
	};
}

#endif
