#include "scene.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "core/transform_node.h"
#include "collisionshape.h"
#include "physical.h"
namespace Scene {

Physical::Physical( 	Core::TransformNode* _node,
						CollisionShape* _shape,
						float _friction,
						float _restitution,
						float _linearDamping,
						float _angularDamping ) : 
	transform( _node ),
	shape( _shape ),
	friction( _friction ),
	restitution( _restitution ),
	linearDamping( _linearDamping ),
	angularDamping( _angularDamping )
{
	btRigidBody::btRigidBodyConstructionInfo rbInfo( 0.0f, this, shape->getBTCollisionShape() );
	rbInfo.m_friction = friction;
	rbInfo.m_restitution = restitution;
	rbInfo.m_linearDamping = linearDamping;
	rbInfo.m_angularDamping = angularDamping;
	body = CORE_NEW btRigidBody( rbInfo );
}

Physical::Physical( 	Physical::INTERNAL,
						Core::TransformNode* _node,
						CollisionShape* _shape,
						float _friction,
						float _restitution,
						float _linearDamping,
						float _angularDamping ) : 
	transform( _node ),
	shape( _shape ),
	friction( _friction ),
	restitution( _restitution ),
	linearDamping( _linearDamping ),
	angularDamping( _angularDamping ) {
}

Physical::~Physical() {
	CORE_DELETE body;
}

void Physical::updateTransform() {
	// TODO wake Bullet? let it know the transform has changed
}

void Physical::getWorldTransform( btTransform& worldTrans ) const {
	const Math::Matrix4x4& matrix = transform->getWorldMatrix();

	worldTrans.setFromOpenGLMatrix( &matrix[0] );
}

void Physical::setWorldTransform(const btTransform& worldTrans) {
	auto& origin = worldTrans.getOrigin();
	auto& orient = worldTrans.getRotation();
	// TODO inverse parent (if any) to transform bullet transform
	// to parent local space
	transform->setLocalPosition( Math::Vector3( origin[0], origin[1], origin[2] ) );
	transform->setLocalOrientation( Math::Quaternion( orient[0], orient[1], orient[2], orient[3] ) );
}

}