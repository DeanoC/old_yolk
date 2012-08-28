#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "core/debug_render.h"
#include "physical.h"
#include "physicsworld.h"

namespace Scene {

class BulletDebugDraw : public btIDebugDraw {
public:
	BulletDebugDraw() : mode(DBG_NoDebug) {}

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& colour) override {
		Core::g_pDebugRender->worldLine( Core::RGBAColour(colour[0],colour[1],colour[2],1), Math::Vector3(from[0],from[1],from[2]), Math::Vector3(to[0],to[1],to[2]) );	
	}

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& colour) override {
		const btVector3 to(PointOnB + normalOnB*distance);
		Core::g_pDebugRender->worldLine( Core::RGBAColour(colour[0],colour[1],colour[2],1), Math::Vector3(PointOnB[0],PointOnB[1],PointOnB[2]), Math::Vector3(to[0],to[1],to[2]) );	
	}

	void reportErrorWarning(const char* text) override {
//		DbgStringOut( text );
	}

	void draw3dText( const btVector3& location, const char* textString ) override {
		// TODO
	}

	void setDebugMode(int mode) override { this->mode = mode; }
	int getDebugMode() const override { return this->mode; }

private:
	int mode;
};

PhysicsWorld::PhysicsWorld() :
	physicsDebugMode(0)
{
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = CORE_NEW btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = CORE_NEW btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = CORE_NEW btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = CORE_NEW btSequentialImpulseConstraintSolver;

	dynamicsWorld = CORE_NEW btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);

	debugDraw  = CORE_NEW BulletDebugDraw();
	debugDraw->setDebugMode( 0 );
	dynamicsWorld->setDebugDrawer( debugDraw );
	//nextPhysicsDebugMode();

	dynamicsWorld->setGravity(btVector3(0,-9.8f,0));
}

PhysicsWorld::~PhysicsWorld() {

	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	for (auto i = dynamicsWorld->getNumCollisionObjects()-1; i >= 0 ; i--) {
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			CORE_DELETE body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject( obj );
		CORE_DELETE obj;
	}

	CORE_DELETE debugDraw;

	//delete dynamics world
	CORE_DELETE dynamicsWorld;

	//delete solver
	CORE_DELETE solver;

	//delete broadphase
	CORE_DELETE overlappingPairCache;

	//delete dispatcher
	CORE_DELETE dispatcher;

	CORE_DELETE collisionConfiguration;
}

void PhysicsWorld::doSim( float delta ) {
	dynamicsWorld->stepSimulation( delta, 7, btScalar(1.f)/btScalar(120.f) );
	dynamicsWorld->debugDrawWorld();
}

btDynamicsWorld* PhysicsWorld::getDynamicsWorld() const {
	return dynamicsWorld;
}

void PhysicsWorld::nextPhysicsDebugMode() {
	switch( physicsDebugMode ) {
		case 0:
			physicsDebugMode = btIDebugDraw::DBG_DrawWireframe;
			break;
		case btIDebugDraw::DBG_DrawWireframe:
			physicsDebugMode = btIDebugDraw::DBG_DrawContactPoints;
			break;
		case btIDebugDraw::DBG_DrawContactPoints:
			physicsDebugMode = 0;
			break;
	}
	debugDraw->setDebugMode( physicsDebugMode );
//		btIDebugDraw::DBG_DrawWireframe |
//		btIDebugDraw::DBG_DrawAabb |
//		btIDebugDraw::DBG_DrawContactPoints |
		 //btIDebugDraw::DBG_DrawText |
		//btIDebugDraw::DBG_DrawConstraintLimits |
//		btIDebugDraw::DBG_DrawConstraints |
//		btIDebugDraw::DBG_DrawNormals |

}
void PhysicsWorld::addPhysical( const std::shared_ptr<Physical>& physical ) {
	getDynamicsWorld()->addRigidBody( physical->getRigidBody() );
}

void PhysicsWorld::removePhysical( const std::shared_ptr<Physical>& physical ) {
	getDynamicsWorld()->removeRigidBody( physical->getRigidBody() );
}

}