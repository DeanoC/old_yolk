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

void Physical::getWorldTransform( btTransform& worldTrans ) const {
	const Math::Matrix4x4 rm = transform->getWorldMatrix();
	const Math::Vector3 xvec = Math::GetXAxis( rm );
	const Math::Vector3 yvec = Math::GetYAxis( rm );
	const Math::Vector3 zvec = Math::GetZAxis( rm );
	const Math::Vector3 pos = Math::GetTranslation( rm );

	btMatrix3x3 basis(	xvec.x, yvec.x, zvec.x,
						xvec.y, yvec.y, zvec.y,
						xvec.z, yvec.z, zvec.z );
	worldTrans.setOrigin( btVector3(pos.x, pos.y, pos.z) );
	worldTrans.setBasis( basis );
}

void Physical::setWorldTransform(const btTransform& worldTrans) {
	// TODO flip?
	auto& origin = worldTrans.getOrigin();
	auto& basis = worldTrans.getBasis();
	btVector3 xvec = basis.getRow(0);
	btVector3 yvec = basis.getRow(1);
	btVector3 zvec = basis.getRow(2);

	Math::Matrix4x4 myBasis = Math::IdentityMatrix();
	Math::SetXAxis( myBasis, Math::Vector3(xvec[0], xvec[1], xvec[2]) );
	Math::SetYAxis( myBasis, Math::Vector3(yvec[0], yvec[1], yvec[2]) );
	Math::SetZAxis( myBasis, Math::Vector3(zvec[0], zvec[1], zvec[2]) );

	// TODO inverse parent (if any) to transform bullet transform
	// to parent local space
	transform->setLocalPosition( Math::Vector3( origin[0], origin[1], origin[2] ) );
	transform->setLocalOrientation( Math::CreateRotationQuat(myBasis) );
}
void Physical::syncBulletTransform() {
	btTransform bt;
	getWorldTransform( bt );
	body->setWorldTransform( bt );
}

}