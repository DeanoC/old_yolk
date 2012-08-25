#include "scene.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "collisionshape.h"
#include "dynamicphysical.h"
namespace Scene {
DynamicPhysical::DynamicPhysical( Core::TransformNode* _node,
							CollisionShape* _shape,
							float _mass, 
							Math::Vector3 _localInertia,
							float _friction,
							float _restitution,
							float _linearDamping,
							float _angularDamping ) :
	Physical( DERIVED, _node, _shape, _friction, _restitution, _linearDamping, _angularDamping ),
	mass( _mass ), 
	localInertia( _localInertia ) {

	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, this, shape->getBTCollisionShape() );
	rbInfo.m_localInertia = btVector3(localInertia.x, localInertia.y, localInertia.z);
	rbInfo.m_friction = friction;
	rbInfo.m_restitution = restitution;
	rbInfo.m_linearDamping = linearDamping;
	rbInfo.m_angularDamping = angularDamping;
	body = new btRigidBody( rbInfo );
}

void DynamicPhysical::setMassProps( const float _mass, const Math::Vector3& _localInertia ) {
	mass = _mass;
	localInertia = _localInertia;

	body->setMassProps( mass, btVector3(localInertia.x, localInertia.y, localInertia.z) );
}

}