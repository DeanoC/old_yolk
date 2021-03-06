#include "meshimport.h"

#include "LWScene2GO.h"
#include "lightwaveloader/DynamicsItemPlugin.h"
#include "meshmod/property.h"
#include "meshmod/scenenode.h"
#include "meshops/basicmeshops.h"
#include "core/aabb.h"

#include "scene/dynamicsproperties.h" // usually meshmod doesn't use scene files but this just makes it easier to keep everything shared

float ExtractSphereShape( MeshMod::SceneNodePtr node );
bool ExtractAABB( MeshMod::SceneNodePtr node, Core::AABB& aabb );
bool ExtractCollisionMesh( MeshMod::SceneNodePtr node, std::vector<float>& verts, std::vector<uint32_t>& indices );

void LWDynamicsItem2SceneNode( LightWave::DynamicsItemPlugin* dip, MeshMod::SceneNodePtr scnNode ) {
	using namespace MeshMod;

	int type;
	int shape;
	int massMethod;

	switch( dip->type ) {
		case 0: type = DYNAMICS_TYPE_DYNAMICS; break;
		case 1: type = DYNAMICS_TYPE_STATIC; break;
		case 2: type = DYNAMICS_TYPE_KINEMATICS; break; // kinematics
		case 3: type = DYNAMICS_TYPE_DYNAMICS; break; // TODO parts
		default: type = DYNAMICS_TYPE_DYNAMICS; break;
	}

	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_TYPE, type ) ); 
	switch( dip->shape ) {
		case 0: shape = DYNAMICS_SHAPE_BOX; {
			Core::AABB aabb;
			bool ok = ExtractAABB( scnNode, aabb );
			if( ok ) {
				const Math::Vector3 minAABB = aabb.getMinExtent();
				const Math::Vector3 maxAABB = aabb.getMaxExtent();

				scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_AABB_MIN, minAABB ) );
				scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_AABB_MAX, maxAABB ) );
			}
		} break;	
		case 1: shape = DYNAMICS_SHAPE_SPHERE; {
			float radius = ExtractSphereShape( scnNode );
			scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_SPHERE_RADIUS, radius ) );
		} break;
		case 2: shape = DYNAMICS_SHAPE_CYLINDER; {
			Core::AABB aabb;
			bool ok = ExtractAABB( scnNode, aabb );
			if( ok ) {
				const Math::Vector3 minAABB = aabb.getMinExtent();
				const Math::Vector3 maxAABB = aabb.getMaxExtent();

				scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_AABB_MIN, minAABB ) );
				scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_AABB_MAX, maxAABB ) );
			}
		} break;
	
		case 4: shape = DYNAMICS_SHAPE_CONVEXHULL; break; // TODO
		case 5: shape = DYNAMICS_SHAPE_MESH; {
			// extract vertices and indices from visual mesh (TODO? collision mesh overrides?)
			// we (as of yet at least) don't try and construct a collision mesh if an LWO wasn't
			// backing it
			std::vector<float> verts;
			std::vector<uint32_t> indices;
			bool ok = ExtractCollisionMesh( scnNode,  verts,  indices );
			if( ok ) {
				scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_MESH_VERTICES, verts ) );
				scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_MESH_INDICES, indices ) );
			}
		} break;
		default: shape = DYNAMICS_SHAPE_SPHERE; break;
	}

	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_SHAPE, shape ) );
	switch( dip->massMethod ) {
		case 0: massMethod = DYNAMICS_MASSMETHOD_MASS; break;
		case 1: massMethod = DYNAMICS_MASSMETHOD_DENSITY; break;
		default: massMethod = DYNAMICS_MASSMETHOD_MASS; break;
	}
	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_MASSMETHOD, massMethod ) );

	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_COLMARGIN , dip->collisionMargin ) );
	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_MASS, dip->givenMass ) );
	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_DENSITY, dip->density ) );
	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_FRICTION, dip->friction ) );
	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_RESTITUTION, dip->restitution ) );
	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_LINEARDAMPING, dip->linearDamping ) );
	scnNode->properties.push_back( std::make_shared<Property>( DYNAMICS_ANGULARDAMPING, dip->angularDamping ) );
}


float ExtractSphereShape( MeshMod::SceneNodePtr node ) {
	using namespace MeshMod;
	if( node->getObjectCount() == 0)
		return false;

	Core::AABB aabb;
	for( unsigned int j=0; j < node->getObjectCount(); ++j ) {
		SceneObjectPtr sobj = node->getObject(j);

		if( sobj->getType() == "mesh" || sobj->getType() == "Mesh" ) {
			MeshPtr omesh = std::dynamic_pointer_cast<Mesh>(sobj);
			Core::AABB localAABB;
			MeshOps::BasicMeshOps ops( omesh );
			ops.computeAABB( localAABB );
			aabb.expandBy( localAABB );
		}
	}
	if( aabb.isValid() ) {
		return Math::Length( aabb.getHalfLength() );
	} else {
		return 1.0f;
	}
}

bool ExtractAABB( MeshMod::SceneNodePtr node, Core::AABB& aabb ) {
	using namespace MeshMod;
	if( node->getObjectCount() == 0)
		return false;

	for( unsigned int j=0; j < node->getObjectCount(); ++j ) {
		SceneObjectPtr sobj = node->getObject(j);

		if( sobj->getType() == "mesh" || sobj->getType() == "Mesh" ) {
			MeshPtr omesh = std::dynamic_pointer_cast<Mesh>(sobj);
			Core::AABB localAABB;
			MeshOps::BasicMeshOps ops( omesh );
			ops.computeAABB( localAABB );
			aabb.expandBy( localAABB );
		}
	}
	if( aabb.isValid() ) {
		return true;
	} else {
		return false;
	}
}

bool ExtractCollisionMesh( MeshMod::SceneNodePtr node, std::vector<float>& verts, std::vector<uint32_t>& indices ) {
	using namespace MeshMod;
	if( node->getObjectCount() == 0)
		return false;
	// toda collapse multple objects into a single collision mesh
	for( unsigned int j=0; j < 1/*node->getObjectCount()*/; ++j ) {
		SceneObjectPtr sobj = node->getObject(j);

		if( sobj->getType() == "mesh" || sobj->getType() == "Mesh" ) {
			MeshPtr omesh = std::dynamic_pointer_cast<Mesh>(sobj);
			MeshPtr mesh( omesh->clone() );

			mesh->removeAllSimilarPositions( 1e-5f );
			MeshOps::BasicMeshOps ops( mesh );
			ops.triangulate();

			VertexElementsContainer& vertCon = mesh->getVertexContainer();
			FaceElementsContainer& faceCon = mesh->getFaceContainer();
			PositionVertexElements*	posEle = vertCon.getElements<PositionVertexElements>();
			FaceFaceElements*	faceEle = faceCon.getElements<FaceFaceElements>();
			assert( posEle != 0 );
			assert( faceEle != 0 );

			verts.resize( (*posEle).size() * 3 );
			for( auto fIt = faceEle->cbegin(); fIt != faceEle->cend(); ++fIt ) {
				std::vector<VertexIndex> faceVertexIndices;
				mesh->getFaceVertices( faceEle->distance<FaceIndex>(fIt), faceVertexIndices );
				// ignore points and lines as have no collision position but are valid after triangulation
				assert( faceVertexIndices.size() <= 3 ); 
				if( faceVertexIndices.size() == 3 ) {
					for( auto fvIt = faceVertexIndices.cbegin(); fvIt != faceVertexIndices.cend(); ++fvIt ) {
						verts[ ((*fvIt) * 3) + 0 ] = (*posEle)[ *fvIt ].x;
						verts[ ((*fvIt) * 3) + 1 ] = (*posEle)[ *fvIt ].y;
						verts[ ((*fvIt) * 3) + 2 ] = (*posEle)[ *fvIt ].z;
						indices.push_back( *fvIt );
					}
				}				
			}
			// TODO remove any unused vertices

			return true;
		}
	}

	return false;
}
