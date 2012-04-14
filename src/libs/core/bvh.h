///-------------------------------------------------------------------------------------------------
/// \file	core\bvh.h
///
/// \brief	a bounding volume hierachy of Aaab. 
///
/// \details	
///		Use as a ray casting acceleration and other uses
///
/// \remark	Copyright (c) 2010 Zombie House Studios. All rights reserved.
/// \remark	mailto://deano@zombiehousestudios.com
///
/// \todo	Fill in detailed file description. 
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef WEIRD_CORE_BVH_H
#define WEIRD_CORE_BVH_H


#include "aabb.h"

namespace Core {
	class TransformNode;
	class Frustum;
}



// really need a new namespace for the ray cast stuff...
namespace Core
{

///-------------------------------------------------------------------------------------------------
/// \class	BVH
///
/// \brief	Bvh. 
///
/// \todo	Fill in detailed class description. 
////////////////////////////////////////////////////////////////////////////////////////////////////
class BVH
{
public:
	///-------------------------------------------------------------------------------------------------
	/// \enum	NODE_AXIS
	///
	/// \brief	Dominant axis of each node. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	enum NODE_AXIS
	{
		X_AXIS = 0,	//!< X. 
		Y_AXIS = 1,	//!< Y.
		Z_AXIS = 2,	//!< Z. 
	};


	///-------------------------------------------------------------------------------------------------
	/// \struct	Node
	///
	/// \brief	Bvh Node. 
	///
	/// \todo	Fill in detailed structure descriptions. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Node
	{
		Node() : m_nTris( 0 ), m_nChildren( 0 ) {}

		/// \brief	Query if this object is leaf. 
		/// \return	true if leaf, false if not. 
		bool isLeaf() const { return m_nChildren == 0; }

		AABB		m_AABB;				//!< The aabb of this node
		uint32_t	m_firstIndex;		//!< Zero-based index of the first child or tru
		uint32_t	m_nTris		: 16;	//!< how many triangles
		uint32_t	m_nChildren	: 6;	//!< support for 64! split tree
		NODE_AXIS	m_axis		: 2;	//!< The dominant axis (For SAH splits, not used atm)
	};

public:
	BVH();
	virtual ~BVH() {}

	//! root bounding box, all nodes must be inside this box
	void setRootAABB( const Core::AABB& root );

	//! adds a list of index triangles to the BVH
	void addIndexTriSource( uint32_t numIndices, const void* pIndices, const void* pVertices, 
							bool indexIs32bit, uint32_t sizeOfVertex );

	//! builds the bvh
	virtual void buildLBVH( int splitsPerLevel, int targetPrimsPerLeaf, int maxLvl );

	//! debug draws the entire bvh to the debug renderer
	virtual void drawDebug( const Core::TransformNode* transform, Core::Frustum* frustum );


	Core::poolvector<Node>	m_nodes;	//!< The nodes of the bvh
// TODO made public to quickly get linux build to work. FIXME
//protected:

	///-------------------------------------------------------------------------------------------------
	/// \struct	NodePrimCreator
	///
	/// \brief	Node primitive creator.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	struct NodePrimCreator
	{
		Core::AABB		m_AABB;			//!< The aabb of this node
		Math::Vector3	m_centroid;		//!< The centroid of this node
		uint64_t		m_mortonCode;	//!< The morton code the centroid of his node
	};


protected:
	void recurseLBVHCreate( unsigned int parentIndex, unsigned int first, unsigned int last, 
							int curLvl, uint64_t lastBit );

	Math::Vector3		m_quantSize;				//!< Size of the quant
	Core::vector<NodePrimCreator> m_triAABB;		//!< tmp vector of nodeprim creators extra data
	int					m_lbvhSplitsPerLevel;		//!< The lbvh splits per level
	int					m_lbvhMaxDescentLevel;		//!< The lbvh maximum descent level
	int					m_lbvhTargetPrimsPerLeaf;	//!< The lbvh target prims per leaf
};

}	//namespace Core

#endif
