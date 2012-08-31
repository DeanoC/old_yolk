//!-----------------------------------------------------
//!
//! \file thingfactory.cpp
//!
//!-----------------------------------------------------
#include "localworld.h"
#include "sceneworld.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "scene/hier.h"
#include "scene/dynamicsproperties.h"
#include "scene/meshcolshape.h"
#include "scene/spherecolshape.h"
#include "scene/boxcolshape.h"
#include "scene/cylindercolshape.h"
#include "scene/dynamicphysical.h"
#include "properties.h"
#include "thingfactory.h"

Thing* ThingFactory::createEmptyThing( const ThingId _id ) {
	return CORE_NEW Thing( _id );
}

Thing* ThingFactory::createThingFromHier( Scene::HierPtr hier, const ThingId _id, boost::function< thingDecodeFuncPtr > callback ) {
	using namespace Core;
	using namespace Scene;

	auto nodeCount = hier->getNodeCount();

	Thing* thing = createEmptyThing( _id );

	// single visual for entire hierachy is the norm
	thing->addRenderable( hier );

	// scan nodes looking for properties
	for( auto i = 0; i < nodeCount; ++i ) {
		auto binHandle = hier->getProperties( i );
		if( binHandle ) {
			std::unordered_map<std::string, int> nameMap;
			auto binProp = binHandle->acquire();
			// build fast name map for properties
			for( auto j = 0; j < binProp->getNumProperties(); ++j ) {
				nameMap[ binProp->getName(j) ] = j;
			}			

			if( callback ) {
				callback( thing, nameMap, binProp, hier->getTransformNode(i) );
			}
		}
	}
	return thing;
}

LevelThing* ThingFactory::createLevelFromHier( Scene::HierPtr hier, const ThingId _id, boost::function< levelThingDecodeFuncPtr > callback ) {
	using namespace Core;
	using namespace Scene;

	auto nodeCount = hier->getNodeCount();

	LevelThing* lvl = CORE_NEW LevelThing( _id );

	// single visual for entire hierachy, TODO structure pre-process or at load?
	lvl->addRenderable( hier );

	// scan nodes looking for properties
	for( auto i = 0; i < nodeCount; ++i ) {
		auto binHandle = hier->getProperties( i );
		if( binHandle ) {
			std::unordered_map<std::string, int> nameMap;
			auto binProp = binHandle->acquire();
			// build fast name map for properties
			for( auto j = 0; j < binProp->getNumProperties(); ++j ) {
				nameMap[ binProp->getName(j) ] = j;
			}			

			decodePhysicProperties( lvl, nameMap, binProp, hier->getTransformNode(i) );
			if( callback ) {
				callback( lvl, nameMap, binProp, hier->getTransformNode(i) );
			}

		}
	}
	return lvl;
}


void ThingFactory::decodePhysicProperties( Thing* thing, std::unordered_map<std::string, int>& nameMap, const Core::BinPropertyResourcePtr& binProp, Core::TransformNode * transformNode ) {
	using namespace Core;
	using namespace Scene;

	if( nameMap.find( DYNAMICS_TYPE ) != nameMap.cend() ) {
		CORE_ASSERT( binProp->getType( nameMap[ DYNAMICS_TYPE ] ) == BinProperty::BPT_INT32 );
		CORE_ASSERT( nameMap.find( DYNAMICS_SHAPE ) != nameMap.cend() );
		CORE_ASSERT( nameMap.find( DYNAMICS_MASSMETHOD ) != nameMap.cend() );	
		CORE_ASSERT( binProp->getType( nameMap[ DYNAMICS_SHAPE ] ) == BinProperty::BPT_INT32 );
		CORE_ASSERT( binProp->getType( nameMap[ DYNAMICS_MASSMETHOD ] ) == BinProperty::BPT_INT32 );

		auto dtype = *binProp->getAs<int32_t>( nameMap[DYNAMICS_TYPE] );
		auto dshape = *binProp->getAs<int32_t>( nameMap[DYNAMICS_SHAPE] );

		CollisionShape* colShape;
	//				printf( "dynamics_type (%u) for %s_%i\n", dtype, hierResName.c_str(), i );
	//				printf( "dynamics_shape (%u) for %s_%i\n", dshape, hierResName.c_str(), i );

		// decode collision shape
		switch( dshape ) {
			case DYNAMICS_SHAPE_MESH: {
				colShape = CORE_NEW MeshColShape( binProp );
			}	break;
			case DYNAMICS_SHAPE_BOX: // intentional fall throw
			case DYNAMICS_SHAPE_CYLINDER: {
				Math::Vector3 minAABB = Math::Vector3(0,0,0);
				Math::Vector3 maxAABB = Math::Vector3(0,0,0);
				if( nameMap.find( DYNAMICS_AABB_MIN ) != nameMap.cend() ) {
					auto mif = binProp->getAs<float>( nameMap[DYNAMICS_AABB_MIN] );
					minAABB = Math::Vector3( mif );
				}
				if( nameMap.find( DYNAMICS_AABB_MAX ) != nameMap.cend() ) {
					auto maf = binProp->getAs<float>( nameMap[DYNAMICS_AABB_MAX] );
					maxAABB = Math::Vector3( maf );
				}
				Core::AABB aabb( minAABB, maxAABB );
				if( dshape == DYNAMICS_SHAPE_BOX ) {
					colShape = CORE_NEW BoxColShape( aabb );
				} else if( dshape == DYNAMICS_SHAPE_CYLINDER ) {
					colShape = CORE_NEW CylinderColShape( aabb );
				}
			} break;
			case DYNAMICS_SHAPE_SPHERE: {
				float radius = 1.0f;
				if( nameMap.find( DYNAMICS_SPHERE_RADIUS ) != nameMap.cend() ) {
					radius = *binProp->getAs<float>( nameMap[DYNAMICS_SPHERE_RADIUS] );
				}
				colShape = CORE_NEW SphereColShape( radius );
			} break;
			default:
				LOG(INFO) << "Unknown dynamics_shape " << dshape << "\n";
				colShape = CORE_NEW SphereColShape( 1.0f );
			break;
		}

		auto dfriction = *binProp->getAs<float>( nameMap[DYNAMICS_FRICTION] );
		auto dresti = *binProp->getAs<float>( nameMap[DYNAMICS_RESTITUTION] );
		auto dldamp = *binProp->getAs<float>( nameMap[DYNAMICS_LINEARDAMPING] );
		auto dadamp = *binProp->getAs<float>( nameMap[DYNAMICS_ANGULARDAMPING] );

		// node has physics but of what type
		switch( dtype ) {
			case DYNAMICS_TYPE_STATIC: {
				thing->addPhysical( std::make_shared<Physical>( transformNode, colShape, 
															dfriction, dresti, dldamp, dadamp ) );
			} break;
			case DYNAMICS_TYPE_DYNAMICS: {
				if( dshape == DYNAMICS_SHAPE_MESH ) {
					LOG(INFO) <<"WARNING: Dynamic Mesh are as expensive as a high class hooker!\n";
				}

				float mass = 1.0f;
				if( nameMap.find( DYNAMICS_MASSMETHOD ) != nameMap.cend() ) {
					assert( binProp->getType( nameMap[ DYNAMICS_MASSMETHOD ] ) == BinProperty::BPT_INT32 );
					auto massMethod = *binProp->getAs<int32_t>( nameMap[DYNAMICS_MASSMETHOD] );

					if( massMethod == DYNAMICS_MASSMETHOD_MASS ) {
						assert( binProp->getType( nameMap[ DYNAMICS_MASS ] ) == BinProperty::BPT_FLOAT );
						mass = *binProp->getAs<float>( nameMap[ DYNAMICS_MASS ] );
					} else {
						// density calc goes here
					}
				}

				btVector3 localInertia; 
				colShape->getBTCollisionShape()->calculateLocalInertia( mass, localInertia );

				thing->addPhysical( std::make_shared<DynamicPhysical>( transformNode, colShape, 
											mass, Math::Vector3(localInertia[0], localInertia[1], localInertia[2]), 
											dfriction, dresti, dldamp, dadamp ) );
			} break;
			default: 
				LOG(INFO) << "Unknown dynamics_type " << dtype << "\n";
			break;
		}
			
	}
}