#include "core/core.h"
#include "core/debug_render.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "scene/mesh.h"
#include "scene/boxcolshape.h"
#include "scene/compoundcolshape.h"
#include "voxtree.h"

#include "voxtreephysical.h"

namespace Vox {

TreePhysical::TreePhysical( Core::TransformNode* _transform, const Tree& _tree ) : 
	compoundShape( std::make_shared<Scene::CompoundColShape>() ),
	Scene::Physical( Scene::Physical::DERIVED, _transform, compoundShape ), 
	tree( _tree ) {

	// make a collision rep
	tree.visitLeaves( 
		// cull function
		[this]( const Vox::Tree::VisitHelper& _helper, const Vox::Node& _node, const Core::AABB& _aabb ) -> bool {
			return false; // no culling for static collision rep
		}, 
		// leaf visit function
		[this]( const Vox::Tree::VisitHelper& _helper, const Vox::Node& _node, const Core::AABB& _aabb ) -> void {
			if( _node.type == NodeType::PACKED_BINARY_LEAF ) {
				for( int i = 0; i < 8; i++ ) {
					bool posiOcc = !!(_node.packedBinaryLeaf.occupancy & (1 << i));
					if(  posiOcc == true || ( posiOcc == false && _node.packedBinaryLeaf.falseBrickIndex != 0 ) ) {
						auto subAABB = _helper.getChildBoundingBox( (ChildName) i, _aabb );
						auto cube = getCubeColShape( subAABB );

						this->compoundShape->addChildShape( subAABB.getBoxCenter(), std::static_pointer_cast<Scene::CollisionShape>( cube ) );
					}
				}
			} else {
				auto cube = getCubeColShape( _aabb );
				this->compoundShape->addChildShape( _aabb.getBoxCenter(), std::static_pointer_cast<Scene::CollisionShape>( cube ) );
			}
		}
	);
	shape = compoundShape;
	btRigidBody::btRigidBodyConstructionInfo rbInfo( 0.0f, this, shape->getBTCollisionShape() );
	rbInfo.m_friction = friction;
	rbInfo.m_restitution = restitution;
	rbInfo.m_linearDamping = linearDamping;
	rbInfo.m_angularDamping = angularDamping;
	// use new as btRigidBody has a class specific new
	body = new btRigidBody( rbInfo );
}

std::shared_ptr<Scene::BoxColShape> TreePhysical::getCubeColShape( const Core::AABB _aabb ) {
	// check our box shape cache (we assume cubes so length.x == length.y == length.z )
	auto cube = cubeShapes.find( _aabb.getHalfLength().x );
	if( cube == cubeShapes.end() ) {
		auto cubeObj = std::make_shared<Scene::BoxColShape>( Core::AABB::fromCenterAndHalfLength( Math::Vector3(0,0,0), _aabb.getHalfLength() ) );
		cubeShapes[ _aabb.getHalfLength().x ] = cubeObj;
		return cubeObj;
	}

	return cube->second;
}

}