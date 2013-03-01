#include "scene.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "core/transform_node.h"
#include "collisionshape.h"
#include "physicsensor.h"
namespace Scene {

PhysicSensor::PhysicSensor( Core::TransformNode* _node,
							std::shared_ptr<CollisionShape> _shape ) : 
	transform( _node ),
	shape( _shape )
{
	// use new as btPairCachingGhostObject has a class specific new
	ghost = new btPairCachingGhostObject();
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

void PhysicSensor::update( btDiscreteDynamicsWorld* dynamicsWorld ) {
	collisions.resize(0);

	btBroadphasePairArray& collisionPairs = ghost->getOverlappingPairCache()->getOverlappingPairArray();
  	const int	numObjects=collisionPairs.size();	

	btManifoldArray	manifoldArray;
	bool added;
	for(int i=0;i<numObjects;i++)	{
    	manifoldArray.resize(0);
    	
		const btBroadphasePair& cPair = collisionPairs[i];
		//unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
    	const btBroadphasePair* collisionPair = dynamicsWorld->getPairCache()->findPair(cPair.m_pProxy0,cPair.m_pProxy1);
    	if (!collisionPair) continue;		
		if( !collisionPair->m_algorithm ) continue;

		collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
		
		added = false;
		for (int j=0;j<manifoldArray.size();j++)	{
			btPersistentManifold* manifold = manifoldArray[j];
			// Here we are in the narrowphase, but can happen that manifold->getNumContacts()==0:
			for (int p=0,numContacts=manifold->getNumContacts();p<numContacts;p++) {
				const btManifoldPoint&pt = manifold->getContactPoint(p);
				if (pt.getDistance() < 0.0 ) {
					// How can I be sure that the colObjs are all distinct ? I use the "added" flag.
					collisions.push_back((btCollisionObject*) (manifold->getBody0() == ghost ? manifold->getBody1() : manifold->getBody0()));
					added = true;
					break;
				}	
			}
			if (added) break;
		}
	}	

}

}