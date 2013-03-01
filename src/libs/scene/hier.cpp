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

Hier::Hier() : Renderable( nullptr ) {}

Hier::Hier( const char* pFilename ) :
	Renderable( nullptr )
{
	hieHandle.reset( HieHandle::load( pFilename ) );
	hie = hieHandle.acquire();
	HierarchyFileHeader* header = hie->header.get();

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
				union Tmp { 
					HierarchyNode* p; 
					struct { uint32_t h; uint32_t l; } o; 
				} *child;
				child = (Tmp*)(tree + 1);
				for( uint32_t j=0;j < node->children.p->numChildren;++j) {
					nodeArray[i].addChild( &nodeArray[ (child[j].p - &nodes[0]) ] );
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
void Hier::renderTransparent( RenderContext* context, Pipeline* pipeline ) {
	OwnedMeshes::const_iterator it = ownedMeshes.begin();
	while( it != ownedMeshes.end() ) {
		(*it)->renderTransparent( context, pipeline );
		++it;
	}
}

Hier* Hier::cloneInstance() const {
	Hier* instance = CORE_NEW Hier();
	instance->hie = this->hie;
	instance->numNodes = this->numNodes;
	instance->propertyArray.reset( CORE_NEW_ARRAY ScopedPropertyHandle[ this->numNodes ] );
	instance->matrixArray.reset( CORE_NEW_ARRAY Math::Matrix4x4[ this->numNodes ] );
	instance->nodeArrayMem.reset( CORE_NEW_ARRAY uint8_t[ sizeof(Core::TransformNode) * this->numNodes ] );
	instance->nodeArray = (Core::TransformNode*) &instance->nodeArrayMem[0];
	instance->transformNode = &nodeArray[0];
	for( uint16_t i=0;i < this->numNodes;++i ) {
		CORE_PLACEMENT_NEW( &instance->nodeArray[i] ) Core::TransformNode( this->matrixArray[i] );
	}

	HierarchyFileHeader* header = hie->header.get();

	HierarchyNode* nodes = (HierarchyNode*) (header+1);
	nodes = (HierarchyNode*) Core::alignTo( (uintptr_t) nodes, 8 );

	for( uint16_t i=0;i < this->numNodes;++i ) {
		// create hierachy as it was a creation time (bar transforms), this might be wrong if hierachy has changed during game
		// TODO proper deep copy of children
		HierarchyNode* node = &nodes[i];
		HierarchyTree* tree = node->children.p;
		if( tree ) {
			union Tmp { 
				HierarchyNode* p; 
				struct { uint32_t h; uint32_t l; } o; 
			} *child;
			child = (Tmp*)(tree + 1);
			for( uint32_t j=0;j < node->children.p->numChildren;++j) {
				instance->nodeArray[i].addChild( &instance->nodeArray[ (child[j].p - &nodes[0]) ] );
			}
		}
		instance->nodeArray[i].setLocalPosition( this->nodeArray[i].getLocalPosition() );
		instance->nodeArray[i].setLocalOrientation( this->nodeArray[i].getLocalOrientation() );
		instance->nodeArray[i].setLocalScale( this->nodeArray[i].getLocalScale() );
		instance->nodeArray[i].m_nodeName = this->nodeArray[i].m_nodeName;

		if( node->flags & HNF_PROPERTIES ) {
			instance->propertyArray[i] = this->propertyArray[i];
		}
	}

	instance->ownedMeshes = this->ownedMeshes;
	instance->localAabb = this->localAabb;

	return instance;
}

}