#include "scene.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "core/transform_node.h"
#include "collisionshape.h"
#include "physicsensor.h"
namespace Scene {

PhysicSensor::PhysicSensor( 	Core::TransformNode* _node,
						CollisionShape* _shape ) : 
	transform( _node ),
	shape( _shape )
{
	ghost = CORE_NEW btPairCachingGhostObject();
	ghost->setCollisionShape( shape->getBTCollisionShape() );
	ghost->setCollisionFlags( btCollisionObject::CF_NO_CONTACT_RESPONSE ); // ghost don't affect the world

}

PhysicSensor::~PhysicSensor() {
	CORE_DELETE ghost;
}

void PhysicSensor::getWorldTransform( btTransform& worldTrans ) const {
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

void PhysicSensor::setWorldTransform(const btTransform& worldTrans) {
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
void PhysicSensor::syncBulletTransform() {
	btTransform bt;
	getWorldTransform( bt );
	ghost->setWorldTransform( bt );
}

}