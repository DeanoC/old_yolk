#include "core/core.h"

#include "voxtree.h"

namespace Vox {

template< typename Derived >
Tree<Derived>::Tree( const Core::AABB& _box ) :
	boundingBox( _box ),
	nodeTiles( 1 )
{
	uint32_t rootIndex = nodeTiles.alloc();
	CORE_ASSERT( rootIndex == 0 );
	for( int i = 0;i < 8; ++i ) {
		nodeTiles.get(0).nodes[i].type = NodeType::EMPTY;
	}

	// leaf index 0 is the same as an EMPTY node (useful property for packers)
}

template< typename Derived >
Tree<Derived>::~Tree() {
}
template< typename Derived >
void Tree<Derived>::visitLeaves( NodeCullFunc _cullFunc, LeafVisitConstFunc _leafFunc, bool _removeEmpty ) const {
	const_cast<Tree<Derived>*>(this)->visitLeaves( _cullFunc, _leafFunc, _removeEmpty );
}

template< typename Derived >
void Tree<Derived>::visitLeaves( NodeCullFunc _cullFunc, LeafVisitConstFunc _leafFunc, bool _removeEmpty ) {
	const VisitHelper helper( *this ); 

	std::stack<TileNodeAndAABB> tileStack;

	if( _cullFunc( helper.getRootBoundingBox() ) == CULL_FUNC_RETURN::CULL)
		return;

	TileNodeAndAABB rootItem( 0, helper.getRootBoundingBox() );
	tileStack.push( rootItem );

	while( tileStack.empty() == false ) {
		auto item = tileStack.top(); tileStack.pop();
		CORE_ASSERT( item.index != INVALID_INDEX );
		const NodeTile& nodeTile = helper.getNodeTile( item.index );
		// check nodes children AABB
		for(int i = 0;i < 8; ++i ) {
			const Node& node = nodeTile.nodes[i];
			// early out for empties
			if( _removeEmpty && node.type == NodeType::EMPTY ) {
				continue;
			}

			// cull node?
			const auto bb = helper.getChildBoundingBox( (ChildName)i, item.aabb );
			const auto cullRet = _cullFunc( bb );
			if( cullRet == CULL_FUNC_RETURN::CULL ) {
				continue;
			}
reGoLabal:
			switch( node.type ) {
				case NodeType::NODE: // push its children onto stack
					tileStack.push( TileNodeAndAABB( node.node.nodeTileIndex, bb ));
					break;
				case NodeType::ONLY_CHILD_NODE: {
					// another level to check
					const auto cbb = helper.getChildBoundingBox( (ChildName)node.onlyChildNode.nodeCode, bb );
					if( _cullFunc( cbb ) != CULL_FUNC_RETURN::CULL ) { 
						tileStack.push( TileNodeAndAABB( node.onlyChildNode.nodeTileIndex, cbb ));
					}
					break;
				}
				case NodeType::TWO_CHILD_NODE: {
					// two nodes of a level down to check
					const auto abb = helper.getChildBoundingBox( (ChildName)node.twoChildNode.nodeACode, bb );
					if( _cullFunc( abb ) != CULL_FUNC_RETURN::CULL ) {
						tileStack.push( TileNodeAndAABB( item.index + node.twoChildNode.nodeATileIndex, abb ));
					}
					const auto bbb = helper.getChildBoundingBox( (ChildName)node.twoChildNode.nodeBCode, bb );
					if( _cullFunc( bbb ) != CULL_FUNC_RETURN::CULL ) {
						tileStack.push( TileNodeAndAABB( item.index + node.twoChildNode.nodeBTileIndex, bbb ));
					}
					break;
				}
				case NodeType::CONSTANT_LEAF: {
					// just fake a leaf node as the AABB has already taken care of the size optimization
					Node tmp = node;
					tmp.type = NodeType::LEAF;
					_leafFunc( helper, tmp, bb );
					break;
				}
				case NodeType::LEAF:
				case NodeType::PACKED_BINARY_LEAF:
					_leafFunc( helper, node, bb );
					break;
				default: {
					bool rego = derived()->visit( 	const_cast<VisitHelper&>( helper ), 
													const_cast<Node&>( node ), 
													bb, _cullFunc, _leafFunc );
					if( rego ) goto reGoLabal;
					break;
				}
			}

		}
	}

}

template< typename Derived >
void Tree<Derived>::pack() {
	packNodeAndDescendants( nodeTiles.get(0).nodes[0] );
}

template< typename Derived >
uint32_t Tree<Derived>::splitNode( Node** _node ) {
	// nodes are already split, so can't be split again
	Node* node = *_node;
	CORE_ASSERT( node->type != NodeType::NODE );

	const auto type = node->type;
	const auto nodeIndex = node - &nodeTiles.get(0).nodes[0];

	uint32_t tileIndex = nodeTiles.alloc();
	CORE_ASSERT( tileIndex < (1 << 28) );
	node = (&nodeTiles.get(0).nodes[0]) + nodeIndex; // require after possible array resize
	*_node = node; // update caller
	NodeTile& children = nodeTiles.get( tileIndex );

	switch( type ) {
		case NodeType::EMPTY: {
			// propagate the emptiness down to the children
			for( int i = 0; i < 8; ++i ) { children.nodes[i].type = NodeType::EMPTY; }
			break;
		}
		case NodeType::LEAF: {
			for( int i = 0; i < 8; ++i ) { 
				children.nodes[i].type = NodeType::LEAF;
				children.nodes[i].leaf.leafIndex = node->leaf.leafIndex;
			}			
			break;
		}
		case NodeType::CONSTANT_LEAF: {
			for( int i = 0; i < 8; ++i ) { 
				children.nodes[i].type = NodeType::LEAF;
				children.nodes[i].leaf.leafIndex = node->constantLeaf.leafIndex;
			}
			break;
		}
		case NodeType::PACKED_BINARY_LEAF: {
			for( int i = 0; i < 8; ++i ) { 
				uint32_t packedIndex;
				if( node->packedBinaryLeaf.occupancy & (1 << i) ) {
					packedIndex = node->packedBinaryLeaf.trueLeafIndex;
				} else {
					packedIndex = node->packedBinaryLeaf.falseLeafIndex;			
				}
				// 0 is a special EMPTY index
				if( packedIndex == 0 ) {
					children.nodes[i].type = NodeType::EMPTY;
				} else {
					children.nodes[i].type = NodeType::LEAF;
					int leafIndex = packedIndex;
					children.nodes[i].leaf.leafIndex = leafIndex;
				}
			}
			break;
		}
		case NodeType::ONLY_CHILD_NODE: 
		case NodeType::TWO_CHILD_NODE: {
			uint32_t nodeACode, nodeBCode;
			uint32_t nodeAIndex, nodeBIndex;
			if( node->type == NodeType::ONLY_CHILD_NODE ) {
				nodeACode = node->onlyChildNode.nodeCode;
				nodeBCode = nodeACode;
				nodeAIndex = node->onlyChildNode.nodeTileIndex;
				nodeBIndex = nodeAIndex;
			} else {
				nodeACode = node->twoChildNode.nodeACode;
				nodeBCode = node->twoChildNode.nodeBCode;
				nodeAIndex = (uint32_t)( (int)tileIndex + (int)node->twoChildNode.nodeATileIndex );
				nodeBIndex = (uint32_t)( (int)tileIndex + (int)node->twoChildNode.nodeBTileIndex );
			}
			for( int i = 0; i < 8; ++i ) { 
				if( i == nodeACode ) {
					children.nodes[i].type = NodeType::NODE;
					children.nodes[i].node.nodeTileIndex = nodeAIndex;
				} else if( i == nodeBCode ) {
					children.nodes[i].type = NodeType::NODE;
					children.nodes[i].node.nodeTileIndex = nodeBIndex;
				} else {
					children.nodes[i].type = NodeType::EMPTY;
				}
			}
			break;
		}
		default: 
			derived()->split( _node, tileIndex );
	}
	// now replace the node with a node tile index indicate pointing to its children
	node->type = NodeType::NODE;
	node->node.nodeTileIndex = tileIndex;

	return tileIndex;
}

/// free this node descendants
template< typename Derived >
void Tree<Derived>::freeNodeDescendants( Node& _node ) {
	std::stack<uint32_t> tileStack;
	if( nodeHasChildren(_node) == false )
		return;

	uint32_t tileIndexA;
	uint32_t tileIndexB;
	if( nodeHasChildren(_node) ) {
		tileIndexA = nodeGetChildrenTileIndex( _node, 0, tileIndexB );
		tileStack.push( tileIndexA );
		if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
	}

	while( tileStack.empty() == false ) {
		uint32_t index = tileStack.top(); tileStack.pop();

		NodeTile& tile = nodeTiles.get( index );
		for( int i = 0;i < 8; ++i ) {
			if( nodeHasChildren(tile.nodes[i]) ) {
				tileIndexA = nodeGetChildrenTileIndex( tile.nodes[i], index, tileIndexB );
				tileStack.push( tileIndexA );
				if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
			}
		}
		nodeTiles.erase( index );
	}
	_node.type = NodeType::EMPTY;
}

/// pack this node and descendants
template< typename Derived >
void Tree<Derived>::packNodeAndDescendants( Node& _node ) {
	std::stack<uint32_t> tileStack;
	uint32_t tileIndexA;
	uint32_t tileIndexB;

	bool packHappened = false;
	do { 
		packHappened = false;
		if( nodeHasChildren(_node) ) {
			tileIndexA = nodeGetChildrenTileIndex( _node, 0, tileIndexB );
			if( tileIndexA != INVALID_INDEX ) tileStack.push( tileIndexA );
			if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
		}
		while( tileStack.empty() == false ) {
			uint32_t index = tileStack.top(); tileStack.pop();
			CORE_ASSERT( index != INVALID_INDEX );
			NodeTile& tile = nodeTiles.get( index );
			for( int i = 0;i < 8; ++i ) {
				if( nodeHasChildren(tile.nodes[i]) ) {
					packHappened |= packNode( tile.nodes[i] );

					tileIndexA = nodeGetChildrenTileIndex( tile.nodes[i], index, tileIndexB );
					if( tileIndexA != INVALID_INDEX ) tileStack.push( tileIndexA );
					if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
				}
			}
		}
	} while( packHappened );
}

template< typename Derived >
void Tree<Derived>::setNodeToLeaf( Node& _node, uint32_t _leafIndex ) {
	// free the children if any
	if( nodeHasChildren(_node) ) {
		freeNodeDescendants( _node );
	}

	// replace this node with the brick
	_node.type = NodeType::LEAF;
	_node.leaf.leafIndex = _leafIndex;
}

// insert a brick of _leafIndex as a point of volume into the tree at point _treeSpacePos
template< typename Derived >
bool Tree<Derived>::insertPoint( const Math::Vector3& _treeSpacePos,
							const uint32_t _leafIndex, 
							const float _volume ) {
	VisitHelper _helper( *this );
	// check point intersects the tree itself
	if(  _helper.getRootBoundingBox().intersects( _treeSpacePos ) == false ) {
		return false;
	}

	// root index and code
	for( int i = 0; i < 8; ++i ) {
		uint32_t curIndex = 0;
		uint32_t curCode = i;
		uint32_t index = 0;
		int level = 0;

		Core::AABB boundingBox = _helper.getChildBoundingBox( (ChildName) curCode, _helper.getRootBoundingBox() );

		// descend until we can't (though we will create split as required if we can)
		while( nodeHasChildren( *curNode ) || nodeIsSplitable( *curNode ) ) {
			Node* curNode = &nodeTiles.get(curIndex).nodes[ curCode ];
			// is the points volume bigger than my children volume?
			if( _volume  > _helper.getNodeVolume(level + 1) ) {
				// replace this node
				setNodeToLeaf( *curNode, _leafIndex );
				return true;	
			}
			// find which child would contain the point
			Math::Vector3 centerPoint = boundingBox.getBoxCenter();
			unsigned int code;
			code  = _treeSpacePos[0] < centerPoint[0] ? LEFT : RIGHT;
			code |= _treeSpacePos[1] < centerPoint[1] ? DOWN : UP;
			code |= _treeSpacePos[2] < centerPoint[2] ? NEAR : FAR;
			ChildName cn = (ChildName) code;

			uint32_t tileIndex;
			// still need to go deeper...
			if( nodeHasChildren(*curNode) ) {
				if( curNode->type == NodeType::ONLY_CHILD_NODE || curNode->type == NodeType::TWO_CHILD_NODE ) {
					uint32_t nodeACode, nodeBCode;
					uint32_t nodeAIndex, nodeBIndex;
					if( curNode->type == NodeType::ONLY_CHILD_NODE ) {
						nodeACode = curNode->onlyChildNode.nodeCode;
						nodeBCode = nodeACode;
						nodeAIndex = curNode->onlyChildNode.nodeTileIndex;
						nodeBIndex = nodeAIndex;
					} else {
						nodeACode = curNode->twoChildNode.nodeACode;
						nodeBCode = curNode->twoChildNode.nodeBCode;
						nodeAIndex = (uint32_t)( (int)curIndex + (int)curNode->twoChildNode.nodeATileIndex );
						nodeBIndex = (uint32_t)( (int)curIndex + (int)curNode->twoChildNode.nodeBTileIndex );
					}
					if( nodeACode == code || nodeBCode == code ) {
						// drop another level
						boundingBox = _helper.getChildBoundingBox( cn, boundingBox );
						centerPoint = boundingBox.getBoxCenter();
						code  = _treeSpacePos[0] < centerPoint[0] ? LEFT : RIGHT;
						code |= _treeSpacePos[1] < centerPoint[1] ? DOWN : UP;
						code |= _treeSpacePos[2] < centerPoint[2] ? NEAR : FAR;
						cn = (ChildName) code;
						level++;
						if( nodeACode == code ) {
							tileIndex = nodeAIndex;
						} else {
							tileIndex = nodeBIndex;
						}
					} else {
						// hit the 6 or 7 virtual empty nodes, so requires a split
						tileIndex = splitNode( &curNode );
						if( tileIndex == INVALID_INDEX ) {
							break;
						}
					}
				} else {
					// standard unpacked node
					uint32_t dummy; // we have already accounted for the second tile index if any
					tileIndex = nodeGetChildrenTileIndex( *curNode, curIndex, dummy );
				}
			} else {
				// need to split if possible (resize may occur, pointer may be invalid afterwards)
				tileIndex = splitNode( &curNode );
				if( tileIndex == INVALID_INDEX ) {
					break;
				}
			}

			// let us go round the loop again
			curIndex = tileIndex;
			curCode = code;
			curNode = &_helper.getNodeTile( tileIndex ).nodes[ code ];
			boundingBox = _helper.getChildBoundingBox( cn, boundingBox );
			level++;
		}
	}

	return false;
}

template< typename Derived>
bool Tree<Derived>::packNode( Node& _node ) {

	if(_node.type == NodeType::NODE ) {
		NodeTile& nodeTile = nodeTiles.get( _node.node.nodeTileIndex );

		// --- gather data on this nodes and its children ---
		int emptyCount = 0;
		int constantCount = 0;
		int constSameLeafCount = 0;
		uint32_t constLeaf = INVALID_INDEX;
		int nonLeafCount = 0;
		int leafAOccupancy = 0;
		int unpackedNodeCount = 0;
		uint32_t leafA = INVALID_INDEX;
		uint32_t leafB = INVALID_INDEX;
		uint32_t unpackedNodeA = INVALID_INDEX;
		uint32_t unpackedNodeB = INVALID_INDEX;
		int unpackedNodeACode = 0;
		int unpackedNodeBCode = 0;

		bool threeOrMoreLeaves = false;
		for( int i = 0;i < 8; i++ ) {
			emptyCount += (nodeTile.nodes[i].type == NodeType::EMPTY);			
			if( nodeTile.nodes[i].type == NodeType::LEAF ) {
				if( leafA == INVALID_INDEX ) {
					leafA = nodeTile.nodes[i].leaf.leafIndex;
					leafAOccupancy |= (1 << i);
				} else if( nodeTile.nodes[i].leaf.leafIndex != leafA ) {
					if( leafB == INVALID_INDEX ) {
						leafB = nodeTile.nodes[i].leaf.leafIndex;
					} else if( nodeTile.nodes[i].leaf.leafIndex != leafB ) {
						threeOrMoreLeaves = true;
					}
				} else {
					leafAOccupancy |= (1 << i);
				}
			} else {
				if( nodeTile.nodes[i].type == NodeType::NODE ) {
					if( unpackedNodeCount == 0 ) {
						unpackedNodeA = nodeTile.nodes[i].node.nodeTileIndex;
						unpackedNodeACode = i;
					} else if( unpackedNodeCount == 1 ) {
						unpackedNodeB = nodeTile.nodes[i].node.nodeTileIndex;
						unpackedNodeBCode = i;
					}
					unpackedNodeCount++;
				} else if( nodeTile.nodes[i].type == NodeType::CONSTANT_LEAF ) {
					constantCount++;
					if( constLeaf == INVALID_INDEX ) {
						constLeaf = nodeTile.nodes[i].constantLeaf.leafIndex;
						constSameLeafCount++;
					} else if( constLeaf == nodeTile.nodes[i].constantLeaf.leafIndex ) {
						constSameLeafCount++;
					}
				}

				nonLeafCount++;
			}
		}

		//--- now decide whether it can be packed ---

		// can we make it an empty node
		if (emptyCount == 8) {
			freeNodeDescendants( _node );
			_node.type = NodeType::EMPTY;
			return true;
		}

		if (constSameLeafCount == 8) {
			Node tmp = nodeTile.nodes[0];
			freeNodeDescendants( _node );
			_node = tmp;
			return true;
		}
		
		
		if( unpackedNodeCount == 1 && 
			emptyCount == 7 && 
			unpackedNodeA < (1 << NODE_ONLY_CHILD_INDEX_BIT_SIZE)) {

			// free nodes tile index as we have packed it away
			nodeTiles.erase( _node.node.nodeTileIndex );

			// point the child we have packed
			_node.type = NodeType::ONLY_CHILD_NODE;
			_node.onlyChildNode.nodeCode = unpackedNodeACode;
			_node.onlyChildNode.nodeTileIndex = unpackedNodeA;
			return true;
		}
		if( unpackedNodeCount == 2 && 
			emptyCount == 6 &&
			unpackedNodeA < (1 << NODE_TWO_CHILD_INDEX_BIT_SIZE) && 
			unpackedNodeB < (1 << NODE_TWO_CHILD_INDEX_BIT_SIZE) ) {

			// free nodes tile index as we have packed it away
			nodeTiles.erase( _node.node.nodeTileIndex );

			// point the 2 children we have packed
			_node.type = NodeType::TWO_CHILD_NODE;
			_node.twoChildNode.nodeACode = unpackedNodeACode;
			_node.twoChildNode.nodeATileIndex = unpackedNodeA - getTileIndex( _node );
			_node.twoChildNode.nodeBCode = unpackedNodeBCode;
			_node.twoChildNode.nodeBTileIndex = unpackedNodeB - getTileIndex( _node );
			return true;
		}
		
		// can we make it a constant node
		if( leafA != INVALID_INDEX && 
			leafB == INVALID_INDEX && 
			nonLeafCount == 0) {
			// all nodes are leaves have the same brick
			freeNodeDescendants( _node );
			_node.type = NodeType::CONSTANT_LEAF;
			_node.constantLeaf.leafIndex = leafA;
			return true;
		}
		
		// can we make it a packed binary with false == EMPTY
		if( leafA != INVALID_INDEX &&
			leafA < (1 << NODE_PACKED_BINARY_BIT_SIZE) &&
			leafB == INVALID_INDEX && 
			emptyCount == nonLeafCount ) {
			freeNodeDescendants( _node );
			_node.type = NodeType::PACKED_BINARY_LEAF;
			_node.packedBinaryLeaf.trueLeafIndex = leafA;
			_node.packedBinaryLeaf.falseLeafIndex = 0; // EMPTY
			_node.packedBinaryLeaf.occupancy = leafAOccupancy;
			return true;
		}

		// can we make it a packed binary node
		if( leafA != INVALID_INDEX &&
			leafA < (1 << NODE_PACKED_BINARY_BIT_SIZE) &&
			leafB != INVALID_INDEX && 
			leafB < (1 << NODE_PACKED_BINARY_BIT_SIZE) ) {
			freeNodeDescendants( _node );
			_node.type = NodeType::PACKED_BINARY_LEAF;
			_node.packedBinaryLeaf.trueLeafIndex = leafA;
			_node.packedBinaryLeaf.falseLeafIndex = leafB;
			_node.packedBinaryLeaf.occupancy = leafAOccupancy;
			return true;
		}
		
		//--- get here, its been unable to be packed
	}
	return false;

}

template< typename Derived>
uint32_t Tree<Derived>::getTileIndex( const Node& _node) const  {
	const auto nodeIndex = &_node - &nodeTiles.get(0).nodes[0];
	return nodeIndex / 8; // truncate to nodeTile
}

template< typename Derived>
Tree<Derived>::VisitHelper::VisitHelper( Tree<Derived>& _parent ) : 
	parent( _parent )
{
}

template< typename Derived>
Tree<Derived>::VisitHelper::VisitHelper( const Tree<Derived>& _parent ) : 
	parent( const_cast<Tree&>(_parent) )
{
}

template< typename Derived>
NodeTile& Tree<Derived>::VisitHelper::getNodeTile( const int _index ) {
	return parent.nodeTiles.get( _index );
}

template< typename Derived>
const NodeTile& Tree<Derived>::VisitHelper::getNodeTile( const int _index ) const {
	return parent.nodeTiles.get( _index );
}

template< typename Derived>
Math::Vector3 Tree<Derived>::VisitHelper::getNodeSize( int _level ) const {

	Core::AABB box = parent.boundingBox;
	Math::Vector3 len = box.getHalfLength();
	// TODO use flog/fexp for all this stuff
	for( int i = 0; i < _level; ++i ) {
		len = len / 2;
	}

	return len * 2;
}

template< typename Derived>
Core::AABB Tree<Derived>::VisitHelper::getChildBoundingBox( const ChildName _childName, const Core::AABB& _parent ) const {
	// size is easy same for all, position is ordered
	auto c = _parent.getBoxCenter();
	auto hl = _parent.getHalfLength();

	Core::AABB box;
	box.expandBy( c );

	switch( _childName ) {
		case ChildName::LUN: box.expandBy( c + Math::Vector3( -hl[0], +hl[1], -hl[2]) ); break;
		case ChildName::RUN: box.expandBy( c + Math::Vector3( +hl[0], +hl[1], -hl[2]) ); break;
		case ChildName::LDN: box.expandBy( c + Math::Vector3( -hl[0], -hl[1], -hl[2]) ); break;
		case ChildName::RDN: box.expandBy( c + Math::Vector3( +hl[0], -hl[1], -hl[2]) ); break;
		case ChildName::LUF: box.expandBy( c + Math::Vector3( -hl[0], +hl[1], +hl[2]) ); break;
		case ChildName::RUF: box.expandBy( c + Math::Vector3( +hl[0], +hl[1], +hl[2]) ); break;
		case ChildName::LDF: box.expandBy( c + Math::Vector3( -hl[0], -hl[1], +hl[2]) ); break;
		case ChildName::RDF: box.expandBy( c + Math::Vector3( +hl[0], -hl[1], +hl[2]) ); break;
	}
	return box;
}

template< typename Derived>
bool Tree<Derived>::nodeHasChildren( const Node& _node ) {
	switch( _node.type ) {
	case NodeType::NODE:
	case NodeType::ONLY_CHILD_NODE:
	case NodeType::TWO_CHILD_NODE:
		return true;
	case NodeType::EMPTY:
	case NodeType::LEAF:
	case NodeType::CONSTANT_LEAF:
	case NodeType::PACKED_BINARY_LEAF:
		return false;
	default:
		return Derived::nodeHasChildren( _node );
	}
}

template< typename Derived>
bool Tree<Derived>::nodeIsSplitable( const Node& _node ) {
	switch( _node.type ) {
	case NodeType::NODE:
		return false;
	case NodeType::LEAF:
		return false; // TODO

	case NodeType::EMPTY:
	case NodeType::CONSTANT_LEAF:
	case NodeType::PACKED_BINARY_LEAF:
	case NodeType::ONLY_CHILD_NODE:
	case NodeType::TWO_CHILD_NODE:
		return true;
	default:
		return Derived::nodeIsSplitable( _node );
	}
}

template< typename Derived>
bool Tree<Derived>::nodeHasDescendants( const Node& _node )  {
	switch( _node.type ) {
	case NodeType::NODE:
	case NodeType::ONLY_CHILD_NODE:
	case NodeType::TWO_CHILD_NODE:
	case NodeType::PACKED_BINARY_LEAF:
	case NodeType::CONSTANT_LEAF:
		return true;

	case NodeType::EMPTY:
	case NodeType::LEAF:
		return false;
	default:
		return Derived::nodeHasDescendants( _node );
	}
}

template< typename Derived>
uint32_t Tree<Derived>::nodeGetChildrenTileIndex( const Node& _node, const uint32_t _thisIndex, uint32_t& _outSecondChildTile ) {
	_outSecondChildTile = INVALID_INDEX;
	switch( _node.type ) {
	case NodeType::NODE:
		return _node.node.nodeTileIndex;
	case NodeType::ONLY_CHILD_NODE:
		return _node.onlyChildNode.nodeTileIndex;
	case NodeType::TWO_CHILD_NODE:
		_outSecondChildTile = _thisIndex + _node.twoChildNode.nodeBTileIndex;
		return _thisIndex + _node.twoChildNode.nodeATileIndex;

	case NodeType::EMPTY:
	case NodeType::LEAF:
	case NodeType::CONSTANT_LEAF:
	case NodeType::PACKED_BINARY_LEAF:
		CORE_ASSERT(false); return INVALID_INDEX;

	default:
		return Derived::nodeGetChildrenTileIndex( _node, _thisIndex, _outSecondChildTile );
	}
}


}