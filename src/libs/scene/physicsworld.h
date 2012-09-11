#ifndef YOLK_SCENE_PHYSICS_WORLD_H_
#define YOLK_SCENE_PHYSICS_WORLD_H_ 1

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btDynamicsWorld;
class btCollisionShape;
class btGhostPairCallback;
class BulletColShape;

namespace Scene {
	class BulletDebugDraw;
	class Physical;
	class PhysicSensor;

	class PhysicsWorld {
	public:
		PhysicsWorld();
		~PhysicsWorld();

		void doSim( float delta );

		void add( const std::shared_ptr<Physical>& physical, uint16_t _collisionType, uint16_t _collisionFlags );
		void remove( const std::shared_ptr<Physical>& physical );

		void add( const std::shared_ptr<PhysicSensor>& sensor, uint16_t _collisionType, uint16_t _collisionFlags );
		void remove( const std::shared_ptr<PhysicSensor>& sensor );

		btDynamicsWorld* getDynamicsWorld() const;
		void nextPhysicsDebugMode();

	protected:
		btDefaultCollisionConfiguration* 	collisionConfiguration;
		btCollisionDispatcher* 				dispatcher;
		btBroadphaseInterface* 				overlappingPairCache;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* 			dynamicsWorld;
		btGhostPairCallback* 				ghostPairCallback;
		BulletDebugDraw*					debugDraw;

		std::vector<std::shared_ptr<PhysicSensor>>	sensors;

		int									physicsDebugMode;
		double								curTime;

	};
}
#endif