// 
//  bvhsah.cpp
//  deano
//  
//  Created by Deano on 2010.
//  Copyright 2010 Zombie House Studios. All rights reserved.
// 

#include "core/core.h"
#include "transform_node.h"
#include "frustum.h"
#include "bvh.h"

namespace Core {

	BVH::BVH() {
	}

	void BVH::setRootAABB( const Core::AABB& root ) {
		m_nodes.clear();

		m_nodes.push_back( Node() );
		m_nodes[0].m_AABB = root;

		const Math::Vector3& minExt = Math::Abs( m_nodes[0].m_AABB.getMinExtent() );
		const Math::Vector3& maxExt = Math::Abs( m_nodes[0].m_AABB.getMaxExtent() );

		m_quantSize.x = (minExt.x > maxExt.x) ? minExt.x : maxExt.x;
		m_quantSize.y = (minExt.y > maxExt.y) ? minExt.y : maxExt.y;
		m_quantSize.z = (minExt.z > maxExt.z) ? minExt.z : maxExt.z;

		m_triAABB.resize(0);			
	}

	void BVH::addIndexTriSource( uint32_t numIndices, const void* pIndices, const void* pVertices, bool indexIs32bit, uint32_t sizeOfVertex ) {
		CORE_ASSERT( m_nodes.size() > 0 );

		// produce temporaily the m_AABB of each triangle
		const uint32_t startIndex = (uint32_t) m_triAABB.size();
		m_triAABB.resize( startIndex + (numIndices / 3) );

		const uint16_t* pIndices16 = (uint16_t*) pIndices;
		const uint32_t* pIndices32 = (uint32_t*) pIndices;

		for( uint32_t i = 0; i < numIndices; i+=3 ) {
			uint32_t i0, i1, i2;
			if( indexIs32bit ) {
				i0 = pIndices32[i + 0];
				i1 = pIndices32[i + 1];
				i2 = pIndices32[i + 2];
			} else {
				i0 = pIndices16[i + 0];
				i1 = pIndices16[i + 1];
				i2 = pIndices16[i + 2];
			}

			Math::Vector3 v0 = Math::Vector3( (float*)(((const char*)pVertices) + sizeOfVertex * i0) );
			Math::Vector3 v1 = Math::Vector3( (float*)(((const char*)pVertices) + sizeOfVertex * i1) );
			Math::Vector3 v2 = Math::Vector3( (float*)(((const char*)pVertices) + sizeOfVertex * i2) );

			NodePrimCreator& c = m_triAABB[ startIndex +(i/3) ];
			c.m_AABB.expandBy( v0 );
			c.m_AABB.expandBy( v1 );
			c.m_AABB.expandBy( v2 );
			c.m_centroid = (v0 + v1 + v2) * (1.0f / 3.0f);
			c.m_mortonCode = Math::MortonCurve( c.m_centroid, m_quantSize );

		}
	}

	// TODO move this, GCC does allow local structures to be passed to STL
	// was in buildBVH
	struct MortonPredicate {
		bool operator()( const BVH::NodePrimCreator& a, const BVH::NodePrimCreator& b ) {
			return a.m_mortonCode < b.m_mortonCode;
		}
	};
	void BVH::buildLBVH( int splitsPerLevel, int targetPrimsPerLeaf, int maxLvl ) {
		MortonPredicate mortonPred;

		// build it into a Linear BVH, fast to compute but not great
		// first sort by morton curve this gives a simple spatial subvision by diving
		// each node into N subnodes (8=octree). This can be done extremely fast but isn't
		// as good for raytracing as a true SAH
		std::sort( m_triAABB.begin(), m_triAABB.end(), mortonPred );
		
		m_lbvhSplitsPerLevel = splitsPerLevel;
		m_lbvhTargetPrimsPerLeaf = targetPrimsPerLeaf;
		m_lbvhMaxDescentLevel = maxLvl;
		recurseLBVHCreate( 0, 0, (uint32_t) m_triAABB.size(), 0, (1ULL << 63) );
		
		LOG(INFO) << "BVH with " << m_nodes.size() << " nodes\n";

		m_triAABB.resize( 0 );
	}

	void BVH::recurseLBVHCreate( unsigned int parentIndex, unsigned int first, unsigned int last, int curLvl, uint64_t curBit ) {
		const unsigned int realFirst = first;

		// a fully populated tree will have m_lbvhSplitsPerLevel child nodes

		// we need to find the high bit set which will define this level
		const NodePrimCreator& fnpc = m_triAABB[ first ];
		const NodePrimCreator& lnpc = m_triAABB[ last -1 ];
		while( (fnpc.m_mortonCode & curBit) != curBit ){
			curBit >>= 1;
		}

		// we need to find out how many triangles belong to this level
		// and then distrubute them between the m_lbvhSplitsPerLevel sub nodes

		// count prims until we change morton code bit and therefore level
		while( first < last ) {
			const NodePrimCreator& npc = m_triAABB[ first ];
//			if( (npc.m_mortonCode & curBit) == 0 ) {
//				break;
//			}
			++first;
		}

		// important counts
		const unsigned int numThisLevel = first - realFirst;
		const unsigned int iNumPerNode = numThisLevel / m_lbvhSplitsPerLevel;
		const bool isLeafNode =		(numThisLevel < (unsigned int) m_lbvhTargetPrimsPerLeaf) ||
									(numThisLevel < (unsigned int) m_lbvhSplitsPerLevel) ||
									(curLvl+1 >= m_lbvhMaxDescentLevel);

		if( isLeafNode ) {
			Node* parent = &m_nodes[parentIndex];
			CORE_ASSERT( parent->m_nChildren == 0 );
			parent->m_nTris = numThisLevel;
			parent->m_firstIndex = first; // TODO tri index to data mapping

		} else {
			// resize node pool
			unsigned int firstNodeIndex = (uint32_t) m_nodes.size();

			unsigned int splits = (unsigned int) m_lbvhTargetPrimsPerLeaf;
			if(iNumPerNode < (unsigned int) m_lbvhTargetPrimsPerLeaf) {
				splits = (numThisLevel / (m_lbvhTargetPrimsPerLeaf))+1;
			}

			m_nodes.resize( m_nodes.size() + splits );
			const float numPerNode = (float)numThisLevel / (float)splits; // float to handle fractionals

			// now grab pointer (node pool may have invalidated pointers)
			Node* parent = &m_nodes[parentIndex];
			CORE_ASSERT( parent->m_nChildren == 0 );
			parent->m_firstIndex = firstNodeIndex;
			parent->m_nChildren = splits;

			// now split, work out size of the m_AABB of each split
			// and the recurse
			first = realFirst;
			float runningTotal = 0.f;
			for( unsigned int i=0; i < splits; ++i ) {
				Node* child = &m_nodes[ firstNodeIndex + i ];
				int numToDo = (int) floorf(runningTotal + numPerNode) - (int)floorf(runningTotal);

				for( int j=0; j < numToDo; ++j ) {
					child->m_AABB.unionWith( m_triAABB[ first + j ].m_AABB );
				}
				recurseLBVHCreate( firstNodeIndex + i, first, first + numToDo, curLvl+1, curBit>>1 );

				runningTotal += numPerNode;
				first = first + numToDo;
			}
		}
	}

	void BVH::drawDebug( const Core::TransformNode* transform, Core::Frustum* frustum ) {
		std::stack< Node* > nodeStack;
		std::stack< float > levelStack;
		nodeStack.push( &m_nodes[0] );
		levelStack.push(0.f);

		// for culling to work we want to move the frstum into local bvh space
		Frustum localFrustum( transform->getWorldTransform() * frustum->m_Matrix );
		while( !nodeStack.empty() ) {
			Node* node = nodeStack.top(); nodeStack.pop();
			float level = levelStack.top(); levelStack.pop();

			bool hidden = false;
			Core::RGBAColour visLeaf(1,1.f - ((level+3)/m_lbvhMaxDescentLevel),0, 0.7f);
			Core::RGBAColour visNode(1,1.f - ((level+3)/m_lbvhMaxDescentLevel),1, 0.3f);
			Core::RGBAColour hidLeaf(0,0.5f,0.5f - ((level+3)/m_lbvhMaxDescentLevel),0.7f);
			Core::RGBAColour hidNode(0,0.5f,0.5f - ((level+3)/m_lbvhMaxDescentLevel),0.3f);

			Core::RGBAColour col;
			const Core::AABB& waabb = node->m_AABB;
			if(localFrustum.cullAABB( waabb ) != Frustum::OUTSIDE ) {
				if( node->isLeaf() ) {
					col = visLeaf;
				} else {
					col = visNode;
				}
			} else {
//				hidden = true;
				if( node->isLeaf() ) {
					col = hidLeaf;
				} else {
					col = hidNode;
				}
			}

			if( !hidden ) {
				waabb.drawDebug( col, transform->getWorldTransform() );
				for( unsigned int i=0;i < node->m_nChildren;++i ){
					nodeStack.push( &m_nodes[node->m_firstIndex+i] );
					levelStack.push( level );
				}
			}
		}
	}
}
