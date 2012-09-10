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

		void add( const std::shared_ptr<Physical>& physical );
		void remove( const std::shared_ptr<Physical>& physical );

		void add( const std::shared_ptr<PhysicSensor>& sensor );
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
		int physicsDebugMode;

		double curTime;

	};
}
#endif