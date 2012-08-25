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
//		DbgWorldLine( 	(const float*) &from[0],
//						(const float*) &to[0],
//						convertColour( colour ) );
		using namespace Core;
		using namespace Math;
		g_pDebugRender->worldLine( RGBAColour(colour[0],colour[1],colour[2],1), Vector3(from[0],from[1],from[2]), Vector3(to[0],to[1],to[2]) );	
	}

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& colour) override {
		const btVector3 to(PointOnB + normalOnB*distance);

//		DbgWorldLine( 	(const float*) &PointOnB[0],
//						(const float*) &to[0],
//						convertColour( colour ) );
		using namespace Core;
		using namespace Math;
		g_pDebugRender->worldLine( RGBAColour(colour[0],colour[1],colour[2],1), Vector3(PointOnB[0],PointOnB[1],PointOnB[2]), Vector3(to[0],to[1],to[2]) );	
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
	uint32_t convertColour( const btVector3& colour ) {
		//workaround to bullet's inconsistent debug colors which are either from 0.0 - 1.0 or from 0.0 - 255.0
		// from irrelict post by randomMesh http://irrlicht.sourceforge.net/forum/viewtopic.php?t=38289 
		btVector3 adjCol = colour;
		if( colour[0] <= 1.0f && colour[0] > 0.0f)
			adjCol[0] = colour[0] * 255.0f;
		if( colour[1] <= 1.0f && colour[1] > 0.0f)
			adjCol[1] = colour[1] * 255.0f;
		if( colour[2] <= 1.0f && colour[2] > 0.0f )
			adjCol[2] = colour[2] * 255.0f;

		// TODO clamp

		return (uint32_t)( ((uint32_t)adjCol[0] << 16) | ((uint32_t)adjCol[1] << 8) | (uint32_t)adjCol[2] );
	}

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
	nextPhysicsDebugMode();
	dynamicsWorld->setDebugDrawer( debugDraw );

	dynamicsWorld->setGravity(btVector3(0,-9.8f,0));

//	timeval tv;
//	gettimeofday( &tv, nullptr );
//	curTime = tv.tv_sec  + ((double)tv.tv_usec / 1e6);

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

//	double lastTime = curTime;
//	timeval tv;
//	gettimeofday( &tv, nullptr );
//	curTime = tv.tv_sec  + ((double)tv.tv_usec / 1e6);
//	float delta = (float)(curTime - lastTime);

//	printf( "delta - %f\n", delta );

	dynamicsWorld->stepSimulation( delta );
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