#include "core/core.h"

#include "voxtree.h"

namespace Vox {

Tree::Tree( const Core::AABB& _box ) :
	boundingBox( _box ),
	nodeTiles( 1 ),
	bricks( 1 )
{
	rootNode.type = NodeType::EMPTY;
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

uint32_t Tree::splitNode( Node& _node ) {
	// nodes are already split, so can't be split again
	CORE_ASSERT( _node.type != NodeType::NODE );

	uint32_t tileIndex = nodeTiles.alloc();
	CORE_ASSERT( tileIndex < (1 << 28) );
	NodeTile& children = nodeTiles.get( tileIndex );

	switch( _node.type ) {
		case NodeType::EMPTY: {
			// propegate the emptiness down to the children
			for( int i = 0; i < 8; ++i ) { children.nodes[i].type = NodeType::EMPTY; }
			break;
		}
		case NodeType::LEAF: {
			CORE_ASSERT( _node.leaf.splitable )
			for( int i = 0; i < 8; ++i ) { 
				children.nodes[i].type = NodeType::LEAF;
				children.nodes[i].leaf.splitable = true;
				children.nodes[i].leaf.brickIndex = _node.leaf.brickIndex;
			}			
			break;
		}
		case NodeType::CONSTANT_LEAF: {
			for( int i = 0; i < 8; ++i ) { 
				children.nodes[i].type = NodeType::LEAF;
				children.nodes[i].leaf.splitable = _node.constantLeaf.splitable;
				children.nodes[i].leaf.brickIndex = _node.constantLeaf.brickIndex;
			}
			break;
		}
		case NodeType::PACKED_BINARY_LEAF: {
			for( int i = 0; i < 8; ++i ) { 
				uint32_t lutIndex;
				if( _node.packedBinaryLeaf.occupancy & (1 << i) ) {
					lutIndex = _node.packedBinaryLeaf.trueBrickIndex;
				} else {
					lutIndex = _node.packedBinaryLeaf.falseBrickIndex;			
				}
				// 0 is a special EMPTY index
				if( lutIndex == 0 ) {
					children.nodes[i].type = NodeType::EMPTY;
				} else {
					children.nodes[i].type = NodeType::LEAF;
					int brickIndex = packedBinaryLUT[ lutIndex ];
					children.nodes[i].leaf.brickIndex = brickIndex;
					// have to get the splitable bit from the brick itself
					// cache is potentially a sad bunny here but split operation
					// are probably quite rare... hmm wonder if worth the replication
					children.nodes[i].leaf.splitable = bricks.get( brickIndex ).splitable;
				}
			}
			break;
		}
		default: 
			CORE_ASSERT( false );
	}
	// now replace the node with a node tile index indicate pointing to its children
	_node.type = NodeType::NODE;
	_node.node.nodeTileIndex = tileIndex;

	return tileIndex;
}
/// free this node descendants
void Tree::freeNodeDescendants( Node& _node ) {
	std::stack<uint32_t> tileStack;
	if( _node.hasChildren() == false )
		return;

	tileStack.push( _node.getChildrenTileIndex() );
	while( tileStack.empty() == false ) {
		uint32_t index = tileStack.top(); tileStack.pop();

		NodeTile& tile = nodeTiles.get( index );
		for( int i = 0;i < 8; ++i ) {
			if( tile.nodes[i].hasChildren() ) {
				tileStack.push( tile.nodes[i].getChildrenTileIndex() );
			}
		}
		nodeTiles.erase( index );
	}
	_node.type = NodeType::EMPTY;
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
		Node* parentNode = nullptr;
		Node* curNode = &rootNode;

		Core::AABB boundingBox = _helper.getRootBoundingBox();

		// descend until we can't (though we will create split as required if we can)
		while( curNode->hasChildren() || curNode->isSplitable() ) {
			// is the points volume bigger than my children volume?
			if( _volume  > _helper.getNodeVolume(level + 1) ) {
				// replace this node
				setNodeToBrick( *curNode, _brickIndex );

				// as one of the parents children has changed, lets
				// see if we can pack it
				if( parentNode != nullptr ) {
					packNode( *parentNode );
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
				tileIndex = curNode->getChildrenTileIndex();
			} else {
				// need to split if possible
				tileIndex = splitNode( *curNode );
				if( tileIndex == INVALID_INDEX ) {
					return;
				}
				CORE_ASSERT( curNode->getChildrenTileIndex() == tileIndex );
			}

			// let us go round the loop again
			parentNode = curNode;
			curNode = &_helper.getNodeTile( tileIndex ).nodes[ code ];
			boundingBox = _helper.getChildBoundingBox( cn, boundingBox );
			level++;
		}
		// failed couldn't insert the point for some reason
	});
	return ret;
}

void Tree::packNode( Node& _node ) {
	// TODO
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
		return true;
	default:
		return false;
	}
}

bool Node::hasDescendants() const {
	switch( type ) {
	case NodeType::NODE:
	case NodeType::PACKED_BINARY_LEAF:
	case NodeType::CONSTANT_LEAF:
		return true;
	default:
		return false;
	}
}

uint32_t Node::getChildrenTileIndex() const {
	// may get more complex if we get multiple node like nodes
	return node.nodeTileIndex;
}


}