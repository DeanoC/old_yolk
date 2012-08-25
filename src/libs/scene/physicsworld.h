#ifndef YOLK_SCENE_PHYSICS_WORLD_H_
#define YOLK_SCENE_PHYSICS_WORLD_H_ 1

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btDynamicsWorld;
class btCollisionShape;
class BulletColShape;

namespace Scene {
	class BulletDebugDraw;
	class Physical;

	class PhysicsWorld {
	public:
		PhysicsWorld();
		~PhysicsWorld();

		void doSim( float delta );

		void addPhysical( const std::shared_ptr<Physical>& physical );
		void removePhysical( const std::shared_ptr<Physical>& physical );

		btDynamicsWorld* getDynamicsWorld() const;
		void nextPhysicsDebugMode();

	protected:
		btDefaultCollisionConfiguration* 	collisionConfiguration;
		btCollisionDispatcher* 				dispatcher;
		btBroadphaseInterface* 				overlappingPairCache;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* 			dynamicsWorld;
		BulletDebugDraw*					debugDraw;
		int physicsDebugMode;

		double curTime;

	};
}
#endif