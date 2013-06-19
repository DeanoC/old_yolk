#include "core/core.h"

#include "voxtree.h"

namespace Vox {

Tree::Tree( const Core::AABB& _box ) :
	boundingBox( _box ),
	nodeTiles( 1 ),
	bricks( 1 )
{
	uint32_t rootIndex = nodeTiles.alloc();
	CORE_ASSERT( rootIndex == 0 );
	// the root node is the first child of the first tileIndex (the other 7 are not used)
	nodeTiles.get(0).nodes[0].type = NodeType::EMPTY;

	// brick index 0 is the same as an EMPTY node (useful property for packers)
	uint32_t emptyIndex = bricks.alloc();
	CORE_ASSERT( emptyIndex == 0 )
}

Tree::~Tree() {
}

void Tree::visit( Tree::DescendConstFunc _func ) const {
	const VisitHelper helper( *this ); 
	_func( helper );
}

void Tree::visit( Tree::DescendFunc _func ) {
	VisitHelper helper( *this ); 
	_func( helper );
}

void Tree::pack() {
	packNodeAndDescendants( nodeTiles.get(0).nodes[0] );
}

uint32_t Tree::splitNode( Node** _node ) {
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
			// propegate the emptiness down to the children
			for( int i = 0; i < 8; ++i ) { children.nodes[i].type = NodeType::EMPTY; }
			break;
		}
		case NodeType::LEAF: {
			CORE_ASSERT( node->leaf.splitable )
			for( int i = 0; i < 8; ++i ) { 
				children.nodes[i].type = NodeType::LEAF;
				children.nodes[i].leaf.splitable = true;
				children.nodes[i].leaf.brickIndex = node->leaf.brickIndex;
			}			
			break;
		}
		case NodeType::CONSTANT_LEAF: {
			for( int i = 0; i < 8; ++i ) { 
				children.nodes[i].type = NodeType::LEAF;
				children.nodes[i].leaf.splitable = node->constantLeaf.splitable;
				children.nodes[i].leaf.brickIndex = node->constantLeaf.brickIndex;
			}
			break;
		}
		case NodeType::PACKED_BINARY_LEAF: {
			for( int i = 0; i < 8; ++i ) { 
				uint32_t packedIndex;
				if( node->packedBinaryLeaf.occupancy & (1 << i) ) {
					packedIndex = node->packedBinaryLeaf.trueBrickIndex;
				} else {
					packedIndex = node->packedBinaryLeaf.falseBrickIndex;			
				}
				// 0 is a special EMPTY index
				if( packedIndex == 0 ) {
					children.nodes[i].type = NodeType::EMPTY;
				} else {
					children.nodes[i].type = NodeType::LEAF;
					int brickIndex = packedIndex;
					children.nodes[i].leaf.brickIndex = brickIndex;
					// have to get the splitable bit from the brick itself
					// cache is potentially a sad bunny here but split operation
					// are probably quite rare... hmm wonder if worth the replication
					children.nodes[i].leaf.splitable = bricks.get( brickIndex ).splitable;
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
			CORE_ASSERT( false );
	}
	// now replace the node with a node tile index indicate pointing to its children
	node->type = NodeType::NODE;
	node->node.nodeTileIndex = tileIndex;

	return tileIndex;
}
/// free this node descendants
void Tree::freeNodeDescendants( Node& _node ) {
	std::stack<uint32_t> tileStack;
	if( _node.hasChildren() == false )
		return;

	uint32_t tileIndexA;
	uint32_t tileIndexB;
	if( _node.hasChildren() ) {
		tileIndexA = _node.getChildrenTileIndex( tileIndexB );
		tileStack.push( tileIndexA );
		if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
	}

	while( tileStack.empty() == false ) {
		uint32_t index = tileStack.top(); tileStack.pop();

		NodeTile& tile = nodeTiles.get( index );
		for( int i = 0;i < 8; ++i ) {
			if( tile.nodes[i].hasChildren() ) {
				tileIndexA = tile.nodes[i].getChildrenTileIndex( tileIndexB );
				tileStack.push( tileIndexA );
				if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
			}
		}
		nodeTiles.erase( index );
	}
	_node.type = NodeType::EMPTY;
}
/// pack this node and descendants
void Tree::packNodeAndDescendants( Node& _node ) {
	std::stack<uint32_t> tileStack;

	uint32_t tileIndexA;
	uint32_t tileIndexB;
	if( _node.hasChildren() ) {
		tileIndexA = _node.getChildrenTileIndex( tileIndexB );
		tileStack.push( tileIndexA );
		if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
	}
	while( tileStack.empty() == false ) {
		uint32_t index = tileStack.top(); tileStack.pop();
		CORE_ASSERT( index != INVALID_INDEX );
		NodeTile& tile = nodeTiles.get( index );
		for( int i = 0;i < 8; ++i ) {
			if( tile.nodes[i].hasChildren() ) {
				packNode( tile.nodes[i] );

				tileIndexA = tile.nodes[i].getChildrenTileIndex( tileIndexB );
				tileStack.push( tileIndexA );
				if( tileIndexB != INVALID_INDEX ) tileStack.push( tileIndexB );
			}
		}
	}
}


uint32_t Tree::allocateBrick( Brick** _outBrick ) {
	uint32_t index = bricks.alloc();
	*_outBrick = &bricks.get( index );
	return index;
}
void Tree::setNodeToBrick( Node& _node, uint32_t _brickIndex ) {
	// free the children if any
	if( _node.hasChildren() ) {
		freeNodeDescendants( _node );
	}

	// replace this node with the brick
	_node.type = NodeType::LEAF;
	// need to look up the brick to see if its splitable			
	_node.leaf.splitable = bricks.get( _brickIndex ).splitable;
	_node.leaf.brickIndex = _brickIndex;
}

// insert a brick of _brickIndex as a point of volume into the tree at point _treeSpacePos
bool Tree::insertPoint( const Math::Vector3& _treeSpacePos,
							const uint32_t _brickIndex, 
							const float _volume ) {
	bool ret = false;
	visit( [&]( VisitHelper& _helper ) {
		// check point intersects the tree itself
		if(  _helper.getRootBoundingBox().intersects( _treeSpacePos ) == false ) {
			return;
		}

		uint32_t index = 0;
		int level = 0;
		// root index and code
		uint32_t parentIndex = 0;
		uint32_t parentCode = 0;
		Node* curNode = &nodeTiles.get(parentIndex).nodes[ parentCode ];
		uint32_t curIndex = parentIndex;
		uint32_t curCode = parentCode;

		Core::AABB boundingBox = _helper.getRootBoundingBox();

		// descend until we can't (though we will create split as required if we can)
		while( curNode->hasChildren() || curNode->isSplitable() ) {
			// is the points volume bigger than my children volume?
			if( _volume  > _helper.getNodeVolume(level + 1) ) {
				// replace this node
				setNodeToBrick( *curNode, _brickIndex );

				// as one of the parents children has changed, lets
				// see if we can pack it
				if( parentIndex != curIndex ) {
					packNode( _helper.getNodeTile( parentIndex ).nodes[ parentCode ] );
				}
				ret = true; // success!
				return;	
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
			if( curNode->hasChildren() ) {
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
						// hit the 6 or 7 virtual empty nodes, so requires a split						tileIndex = splitNode( &curNode );
						tileIndex = splitNode( &curNode );
						if( tileIndex == INVALID_INDEX ) {
							return;
						}
					}
				} else {
					// standard unpacked node
					uint32_t dummy; // we have already accounted for the second tile index if any
					tileIndex = curNode->getChildrenTileIndex( dummy );
				}
			} else {
				// need to split if possible (resize may occur, pointer may be invalid afterwards)
				tileIndex = splitNode( &curNode );
				if( tileIndex == INVALID_INDEX ) {
					return;
				}
			}

			// let us go round the loop again
			parentIndex = curIndex;
			parentCode = curCode;
			curIndex = tileIndex;
			curCode = code;
			curNode = &_helper.getNodeTile( tileIndex ).nodes[ code ];
			boundingBox = _helper.getChildBoundingBox( cn, boundingBox );
			level++;
		}
		// failed couldn't insert the point for some reason
	});
	return ret;
}

void Tree::packNode( Node& _node ) {

	if(_node.type == NodeType::NODE ) {
		NodeTile& nodeTile = nodeTiles.get( _node.node.nodeTileIndex );

		// --- gather data on this nodes and its children ---
		int emptyCount = 0;
		int nonLeafCount = 0;
		int leafBrickAOccupancy = 0;
		int unpackedNodeCount = 0;
		uint32_t leafBrickA = INVALID_INDEX;
		uint32_t leafBrickB = INVALID_INDEX;
		uint32_t unpackedNodeA = INVALID_INDEX;
		uint32_t unpackedNodeB = INVALID_INDEX;
		int unpackedNodeACode = 0;
		int unpackedNodeBCode = 0;

		bool threeOrMoreLeafBricks = false;
		for( int i = 0;i < 8; i++ ) {
			emptyCount += (nodeTile.nodes[i].type == NodeType::EMPTY);			
			if( nodeTile.nodes[i].type == NodeType::LEAF ) {
				if( leafBrickA == INVALID_INDEX ) {
					leafBrickA = nodeTile.nodes[i].leaf.brickIndex;
					leafBrickAOccupancy |= (1 << i);
				} else if( nodeTile.nodes[i].leaf.brickIndex != leafBrickA ) {
					if( leafBrickB == INVALID_INDEX ) {
						leafBrickB = nodeTile.nodes[i].leaf.brickIndex;
					} else if( nodeTile.nodes[i].leaf.brickIndex != leafBrickB ) {
						threeOrMoreLeafBricks = true;
					}
				} else {
					leafBrickAOccupancy |= (1 << i);
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
				}
				nonLeafCount++;
			}
		}

		//--- now decide whether it can be packed ---

		// can we make it an empty node
		if (emptyCount == 8) {
			freeNodeDescendants( _node );
			_node.type = NodeType::EMPTY;
			return;
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
			return;
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
			return;
		}

		// can we make it a constant node
		if( leafBrickA != INVALID_INDEX && 
			leafBrickB == INVALID_INDEX && 
			nonLeafCount == 0) {
			// all nodes are leaves have the same brick
			bool splitable = nodeTile.nodes[0].constantLeaf.splitable;
			freeNodeDescendants( _node );
			_node.type = NodeType::CONSTANT_LEAF;
			_node.constantLeaf.splitable = splitable;
			_node.constantLeaf.brickIndex = leafBrickA;
			return;
		}

		// can we make it a packed binary with false == EMPTY
		if( leafBrickA != INVALID_INDEX &&
			leafBrickA < (1 << PACKED_BINARY_BIT_SIZE) &&
			leafBrickB == INVALID_INDEX && 
			emptyCount == nonLeafCount ) {
			freeNodeDescendants( _node );
			_node.type = NodeType::PACKED_BINARY_LEAF;
			_node.packedBinaryLeaf.trueBrickIndex = leafBrickA;
			_node.packedBinaryLeaf.falseBrickIndex = 0; // EMPTY
			_node.packedBinaryLeaf.occupancy |= leafBrickAOccupancy;
			return;
		}

		// can we make it a packed binary node
		if( leafBrickA != INVALID_INDEX &&
			leafBrickA < (1 << PACKED_BINARY_BIT_SIZE) &&
			leafBrickB != INVALID_INDEX && 
			leafBrickB < (1 << PACKED_BINARY_BIT_SIZE) ) {
			freeNodeDescendants( _node );
			_node.type = NodeType::PACKED_BINARY_LEAF;
			_node.packedBinaryLeaf.trueBrickIndex = leafBrickA;
			_node.packedBinaryLeaf.falseBrickIndex = leafBrickB;
			_node.packedBinaryLeaf.occupancy |= leafBrickAOccupancy;
			return;
		}

		//--- get here, its been unable to be packed
	}

}

uint32_t Tree::getTileIndex( Node& _node) const {
	const auto nodeIndex = &_node - &nodeTiles.get(0).nodes[0];
	return nodeIndex / 8; // truncate to nodeTile
}

Tree::VisitHelper::VisitHelper( Tree& _parent ) : 
	parent( _parent )
{
}

Tree::VisitHelper::VisitHelper( const Tree& _parent ) : 
	parent( const_cast<Tree&>(_parent) )
{
}
NodeTile& Tree::VisitHelper::getNodeTile( const int _index ) {
	return parent.nodeTiles.get( _index );
}

const NodeTile& Tree::VisitHelper::getNodeTile( const int _index ) const {
	return parent.nodeTiles.get( _index );
}

Math::Vector3 Tree::VisitHelper::getNodeSize( int _level ) const {

	Core::AABB box = parent.boundingBox;
	Math::Vector3 len = box.getHalfLength();
	// TODO use flog/fexp for all this stuff
	for( int i = 0; i < _level; ++i ) {
		len = len / 2;
	}

	return len * 2;
}

Core::AABB Tree::VisitHelper::getChildBoundingBox( const ChildName _childName, const Core::AABB& _parent ) const {
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

bool Node::isSplitable() const {
	switch( type ) {
	case NodeType::NODE:
		return false;
	case NodeType::LEAF:
		return leaf.splitable;
	case NodeType::CONSTANT_LEAF:
	case NodeType::PACKED_BINARY_LEAF:
	default:
		return true;
	}
}

bool Node::hasChildren() const {
	switch( type ) {
	case NodeType::NODE:
	case NodeType::ONLY_CHILD_NODE:
	case NodeType::TWO_CHILD_NODE:
		return true;
	default:
		return false;
	}
}

bool Node::hasDescendants() const {
	switch( type ) {
	case NodeType::NODE:
	case NodeType::ONLY_CHILD_NODE:
	case NodeType::TWO_CHILD_NODE:
	case NodeType::PACKED_BINARY_LEAF:
	case NodeType::CONSTANT_LEAF:
		return true;
	default:
		return false;
	}
}

uint32_t Node::getChildrenTileIndex( uint32_t& _outSecondChildTile ) const {
	_outSecondChildTile = INVALID_INDEX;
	switch( type ) {
	case NodeType::NODE:
		return node.nodeTileIndex;
	case NodeType::ONLY_CHILD_NODE:
		return onlyChildNode.nodeTileIndex;
	case NodeType::TWO_CHILD_NODE:
		_outSecondChildTile = twoChildNode.nodeBTileIndex;
		return twoChildNode.nodeATileIndex;
	default:
		return INVALID_INDEX;
	}
}


}