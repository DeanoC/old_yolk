#pragma once
#ifndef YOLK_PROCVOXTREE_H_
#define YOLK_PROCVOXTREE_H_ 1

#include "voxtree.h"

namespace Vox {

enum ProcVoxNodeType {
	PROCEDURAL_NODE = NodeType::USER_NODE_TYPE_START,
};

class ProcVoxTree : public Tree<ProcVoxTree> {
public:
	friend class Tree<ProcVoxTree>;
	ProcVoxTree( const Core::AABB& _box );

protected:
	bool visit( VisitHelper& _helper, 
				Node& _node, 
				const Core::AABB& _aabb, 
				NodeCullFunc _cullFunc, 
				LeafVisitConstFunc _leafFunc );

	void split() const {};


	bool ProcVoxTree::generate( 	VisitHelper& _helper, 
									const Core::AABB& _aabb, 
									NodeCullFunc _cullFunc, 
									LeafVisitConstFunc _leafFunc );
	
	static bool nodeHasChildren( const Node& _node ) { 
		switch( _node.type ) {
			case PROCEDURAL_NODE:
				return false;
			default:
				return false;
		}
	}
	static bool nodeIsSplitable( const Node& _node ) { 
		switch( _node.type ) {
			case PROCEDURAL_NODE:
				return true;
			default:
				return false;
		}
	}
	static bool nodeHasDescendants( const Node& _node ) { 
		switch( _node.type ) {
			case PROCEDURAL_NODE:
				return false;
			default:
				return false;
		}
	}

	static uint32_t nodeGetChildrenTileIndex( const Node& _node, const uint32_t _thisIndex, uint32_t& _outSecondChildTile ) { 
		CORE_ASSERT(false); return INVALID_INDEX; 
	}
};

}

#endif