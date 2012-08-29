//!-----------------------------------------------------
//!
//! \file hieskeleton.cpp
//! a hierachy of transform nodes, usually from disk
//!
//!-----------------------------------------------------

#include "scene.h"
#include "mesh.h"
#include "pipeline.h"

#include "hier.h"

namespace Scene
{

Hier::Hier( const char* pFilename ) :
	Renderable( nullptr )
{
	hieHandle.reset( HieHandle::load( pFilename ) );
	hie = hieHandle.acquire();
	HierarchyFileHeader* header = hie->header.get();

	if( header->flags & HFF_ENVIRONMENT ) {
		environment = std::make_shared<Environment>( Core::BinPropertyResourceHandle::load( pFilename ) );
	}

	HierarchyNode* nodes = (HierarchyNode*) (header+1);
	nodes = (HierarchyNode*) Core::alignTo( (uintptr_t) nodes, 8 );

	numNodes = header->numNodes;
	if( header->numNodes > 0 ) {
		propertyArray.reset( CORE_NEW_ARRAY ScopedPropertyHandle[ header->numNodes ] );
		matrixArray.reset( CORE_NEW_ARRAY Math::Matrix4x4[ header->numNodes ] );
		nodeArrayMem.reset( CORE_NEW_ARRAY uint8_t[ sizeof(Core::TransformNode) * header->numNodes ] );
		nodeArray = (Core::TransformNode*) &nodeArrayMem[0];
		transformNode = &nodeArray[0];

		for( uint16_t i=0;i < header->numNodes;++i ) {
			CORE_PLACEMENT_NEW( &nodeArray[i] ) Core::TransformNode( matrixArray[i] );
		}
		for( uint16_t i=0;i < header->numNodes;++i ) {
			HierarchyNode* node = &nodes[i];
			HierarchyTree* tree = node->children.p;
			if( tree ) {
				uint32_t* indices = (uint32_t*)(tree + 1);
				for( uint32_t j=0;j < node->children.p->numChildren;++j) {
					nodeArray[i].addChild( nodeArray + indices[j] );
				}
			}
			nodeArray[i].setLocalPosition( Math::Vector3( node->pos ) );

			Math::Quaternion quat( node->quat[0], node->quat[1], node->quat[2], node->quat[3] ); 
//TODO			quat = Math::Normalise( quat );
			nodeArray[i].setLocalOrientation( quat );
			nodeArray[i].setLocalScale( Math::Vector3( node->scale ) );
			nodeArray[i].m_nodeName = node->nodeName.p;

			if( node->flags & HNF_PROPERTIES ) {
				propertyArray[i].reset( Core::BinPropertyResourceHandle::load( (std::string(pFilename) + "_" + nodeArray[i].m_nodeName).c_str() ) );
			}

			if( node->type == HNT_MESH ) {
				std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>( node->meshName.p, &nodeArray[i] );
				ownedMeshes.push_back( mesh );
				localAabb.expandBy( mesh->getLocalAabb() );
			}			
		}

	} else {
		// empty hierachy, just create a dummy node
		matrixArray.reset( CORE_NEW_ARRAY Math::Matrix4x4[ 1 ] );
		nodeArrayMem.reset( CORE_NEW_ARRAY uint8_t[ sizeof(Core::TransformNode) * 1 ] );
		nodeArray = (Core::TransformNode*) &nodeArrayMem[0];
		transformNode = &nodeArray[0];

		CORE_PLACEMENT_NEW( getTransformNode() ) Core::TransformNode( matrixArray[0] );
	}
}

Hier::~Hier() {
	propertyArray.reset();
	environment.reset();
	hie.reset();
	ownedMeshes.clear();

	for( uint16_t i=0;i < numNodes;++i ) {
		nodeArray[i].~TransformNode();
	}	
}

void Hier::render( RenderContext* context, Pipeline* pipeline ) {
	OwnedMeshes::const_iterator it = ownedMeshes.begin();
	while( it != ownedMeshes.end() ) {
		(*it)->render( context, pipeline );
		++it;
	}
}

}