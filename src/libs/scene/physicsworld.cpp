#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "core/debug_render.h"
#include "physical.h"
#include "physicsensor.h"
#include "physicsworld.h"

namespace Scene {

// Portable static method: prerequisite call: m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback()); 
static void GetCollidingObjectsInsidePairCachingGhostObject(
					btDiscreteDynamicsWorld* dynamicsWorld,
					btPairCachingGhostObject* pairCachingGhostObject,
					btAlignedObjectArray < btCollisionObject* >& collisionArrayOut )	{

	collisionArrayOut.resize(0);
	if (!pairCachingGhostObject || !dynamicsWorld) return;
	const bool addOnlyObjectsWithNegativeDistance(true);	// With "false" things don't change much, and the code is a bit faster and cleaner...

	//#define USE_PLAIN_COLLISION_WORLD // We dispatch all collision pairs of the ghost object every step (slow)
#ifdef USE_PLAIN_COLLISION_WORLD
	//======================================================================================================
	// I thought this line was no longer needed, but it seems to be necessary (and I believe it's an expensive call):
	dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(m_pairCachingGhostObject->getOverlappingPairCache(), dynamicsWorld->getDispatchInfo(), dynamicsWorld->getDispatcher());
	// Maybe the call can be automatically triggered by some other Bullet call (I'm almost sure I could comment it out in another demo I made long ago...)
	// So by now the general rule is: in real projects, simply comment it out and see if it works!
	//======================================================================================================
	// UPDATE: in dynamic worlds, the line above can be commented out and the broadphase pair can be retrieved through the call to findPair(...) below.
	// In collision worlds probably the above line is needed only if collision detection for all the bodies hasn't been made... This is something
	// I'm still not sure of... the general rule is to try to comment out the line above and try to use findPair(...) and see if it works whenever possible....
	//======================================================================================================
#endif //USE_PLAIN_COLLISION_WORLD
	
	btBroadphasePairArray& collisionPairs = pairCachingGhostObject->getOverlappingPairCache()->getOverlappingPairArray();
  	const int	numObjects=collisionPairs.size();	
	static btManifoldArray	manifoldArray;
	bool added;
	for(int i=0;i<numObjects;i++)	{
    	manifoldArray.resize(0);
    	
#ifdef USE_PLAIN_COLLISION_WORLD
		const btBroadphasePair& collisionPair = collisionPairs[i];
		if (collisionPair.m_algorithm) collisionPair.m_algorithm->getAllContactManifolds(m_manifoldArray);
		else {	// THIS SHOULD NEVER HAPPEN, AND IF IT DOES, PLEASE RE-ENABLE the "call" a few lines above...
			CORE_ASSERT( false && "No collisionPair.m_algorithm - probably m_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(...) must be missing.\n");
		}	
#else // USE_PLAIN_COLLISION_WORLD	
		const btBroadphasePair& cPair = collisionPairs[i];
		//unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
    	const btBroadphasePair* collisionPair = dynamicsWorld->getPairCache()->findPair(cPair.m_pProxy0,cPair.m_pProxy1);
    	if (!collisionPair) continue;		
		if (collisionPair->m_algorithm) collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
		else {	// THIS SHOULD NEVER HAPPEN, AND IF IT DOES, PLEASE RE-ENABLE the "call" a few lines above...
			CORE_ASSERT( false && "No collisionPair.m_algorithm - probably m_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(...) must be missing.\n");	
		}
#endif //USE_PLAIN_COLLISION_WORLD
		
		added = false;
		for (int j=0;j<manifoldArray.size();j++)	{
			btPersistentManifold* manifold = manifoldArray[j];
			// Here we are in the narrowphase, but can happen that manifold->getNumContacts()==0:
			if (addOnlyObjectsWithNegativeDistance)	{
				for (int p=0,numContacts=manifold->getNumContacts();p<numContacts;p++)			{
					const btManifoldPoint&pt = manifold->getContactPoint(p);
					if (pt.getDistance() < 0.0) 	{
						// How can I be sure that the colObjs are all distinct ? I use the "added" flag.
						collisionArrayOut.push_back((btCollisionObject*) (manifold->getBody0() == pairCachingGhostObject ? manifold->getBody1() : manifold->getBody0()));
						added = true;
						break;
					}	
				}
			if (added) break;
			}
			else if (manifold->getNumContacts()>0) {
				collisionArrayOut.push_back((btCollisionObject*) (manifold->getBody0() == pairCachingGhostObject ? manifold->getBody1() : manifold->getBody0()));
				break;
			}
		}
	}	
}


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

	// Needed once to enable ghost objects inside Bullet
	ghostPairCallback = CORE_NEW btGhostPairCallback(); 
	dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback( ghostPairCallback );

	debugDraw  = CORE_NEW BulletDebugDraw();
	debugDraw->setDebugMode( 0 );
	dynamicsWorld->setDebugDrawer( debugDraw );
	//nextPhysicsDebugMode();

	dynamicsWorld->setGravity(btVector3(0,-9.8f,0));
}

PhysicsWorld::~PhysicsWorld() {

	//cleanup in the reverse order of creation/initialization

	// hopefully this list is empty by now...
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
	CORE_DELETE ghostPairCallback;
	CORE_DELETE dynamicsWorld;
	CORE_DELETE solver;
	CORE_DELETE overlappingPairCache;
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

void PhysicsWorld::add( const std::shared_ptr<Physical>& physical ) {
	getDynamicsWorld()->addRigidBody( physical->getRigidBody() );
}

void PhysicsWorld::remove( const std::shared_ptr<Physical>& physical ) {
	getDynamicsWorld()->removeRigidBody( physical->getRigidBody() );
}

void PhysicsWorld::add( const std::shared_ptr<PhysicSensor>& sensor ) {
	// collide with everything but other ghosts
	getDynamicsWorld()->addCollisionObject( sensor->getGhost(),	
						btBroadphaseProxy::SensorTrigger,
						btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger );
}

void PhysicsWorld::remove( const std::shared_ptr<PhysicSensor>& sensor ) {
	getDynamicsWorld()->removeCollisionObject( sensor->getGhost() );
}

}