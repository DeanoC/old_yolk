#pragma once
#ifndef YOLK_SCENE_PHYSIC_SENSOR_H_
#define YOLK_SCENE_PHYSIC_SENSOR_H_ 1

#include "LinearMath/btMotionState.h"
namespace Core {
	class TransformNode;
}

class btPairCachingGhostObject;
class btDiscreteDynamicsWorld;
class btCollisionObject;

namespace Scene {
	class CollisionShape;
	class PhysicSensor : public btMotionState {
	public:
		friend class PhysicsWorld;
		typedef std::vector < btCollisionObject* > CollidedArray;

		explicit PhysicSensor( 	Core::TransformNode* node, std::shared_ptr<CollisionShape> shape );
		virtual ~PhysicSensor();

		const btPairCachingGhostObject* getGhost() const { return ghost; }
		btPairCachingGhostObject* getGhost() { return ghost; }


		Core::TransformNode* getTransformNode() const { return transform; }
		// physical objects normally are driven by bullet, sometimes however its useful to force this transform node on bullet
		void syncBulletTransform();

		// btMotionState interface
		virtual void    getWorldTransform( btTransform& worldTrans ) const override;
		virtual void    setWorldTransform( const btTransform& worldTrans ) override;	

		CollidedArray::const_iterator cbegin() const { return collisions.cbegin(); }
		CollidedArray::const_iterator cend() const { return collisions.cend(); }
		CollidedArray::iterator begin() { return collisions.begin(); }
		CollidedArray::iterator end() { return collisions.end(); }

	protected:
		void update( btDiscreteDynamicsWorld* dynamicsWorld );

		Core::TransformNode* 					transform;
		std::shared_ptr<CollisionShape>			shape;
		btPairCachingGhostObject*				ghost;
		CollidedArray							collisions;
	};

	typedef std::shared_ptr<PhysicSensor>			PhysicSensorPtr;
}

#endif