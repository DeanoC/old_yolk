#pragma once
#ifndef YOLK_SCENE_PHYSICAL_H_
#define YOLK_SCENE_PHYSICAL_H_ 1

#include "LinearMath/btMotionState.h"
namespace Core {
	class TransformNode;
}

class btRigidBody;
namespace Scene {
	class CollisionShape;
	class Physical : public btMotionState {
	public:
		explicit Physical( 	Core::TransformNode* node,
							CollisionShape* shape,
							float _friction = 0.5f,
							float _restitution = 0.f,
							float _linearDamping = 0.f,
							float _angularDamping = 0.f );
		virtual ~Physical();

		const btRigidBody* getRigidBody() const { return body; }
		btRigidBody* getRigidBody() { return body; }

		virtual float getMass() const { return 0.0f; }
		virtual Math::Vector3 getLocalInertia() const { return Math::Vector3(0,0,0); }
		Core::TransformNode* getTransformNode() const { return transform; }
		// physical objects normally are driven by bullet, sometimes however its useful to force this transform node on bullet
		void syncBulletTransform();

		// btMotionState interface
		virtual void    getWorldTransform( btTransform& worldTrans ) const override;
		virtual void    setWorldTransform( const btTransform& worldTrans ) override;	
	protected:
		enum INTERNAL{ DERIVED };

		explicit Physical( 	INTERNAL,
							Core::TransformNode* node,
							CollisionShape* shape,
							float _friction,
							float _restitution,
							float _linearDamping,
							float _angularDamping );
		Core::TransformNode* 					transform;
		std::shared_ptr<CollisionShape>			shape;
		btRigidBody* 							body;
		float 									friction;
		float 									restitution;
		float 									linearDamping;
		float 									angularDamping;
	};

	typedef std::shared_ptr<Physical>			PhysicalPtr;
}

#endif