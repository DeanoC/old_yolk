#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "compoundcolshape.h"

namespace Scene {
	CompoundColShape::CompoundColShape() :
		compoundShape( nullptr ) 
	{
		// use new as btCompoundShape has a class specific new
		compoundShape  = new btCompoundShape();
	}
	CompoundColShape::~CompoundColShape() {
		CORE_DELETE compoundShape;
	}
	
	btCollisionShape* CompoundColShape::getBTCollisionShape() const { 
		return static_cast<btCollisionShape*>(compoundShape); 
	}

	void CompoundColShape::addChildShape( const Math::Vector3& _attachPt, std::shared_ptr<CollisionShape> _shape ) {
		owned.push_back( _shape );

		btTransform localTrans;
		localTrans.setIdentity();
		localTrans.setOrigin( btVector3( _attachPt[0], _attachPt[1], _attachPt[2] ) );
	
		compoundShape->addChildShape( localTrans, _shape->getBTCollisionShape() );
	}

}