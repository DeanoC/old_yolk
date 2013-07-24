#pragma once
#ifndef YOLK_VOXTREE_H_
#define YOLK_VOXTREE_H_ 1

#include "core/aabb.h"
#include "freelist.h" // TODO move to Core

namespace Vox {

//! the normal size a  point like brick will be represented in the voxtree
static const float DEFAULT_LEAF_RESOLUTION		= 1.0f;

static const int NODE_TYPE_BIT_SIZE 			= 4;
static const int NODE_PAYLOAD_BIT_SIZE 			= 28;
static const int NODE_PACKED_BINARY_BIT_SIZE 	= 10;
static const int NODE_LEAF_INDEX_BIT_SIZE	 	= 27;
static const int NODE_INDEX_BIT_SIZE 			= 28;
static const int NODE_ONLY_CHILD_INDEX_BIT_SIZE = 25;
static const int NODE_TWO_CHILD_INDEX_BIT_SIZE = 11;

static const uint32_t INVALID_INDEX				= ~0;

// we only use 4 bits
enum NodeType {
	EMPTY = 0,				//!< nada, technically redundant as CONSTNAT_LEAF could be used
	NODE,					//!< standard node, with an index to a children node tile
	LEAF,					//!< leaf node, has an index to a brick
	CONSTANT_LEAF,			//!< all children use the same brick
	PACKED_BINARY_LEAF,		//!< children only use 2 bricks, so are packed here without a nodetile
	ONLY_CHILD_NODE,		//!< the node only has 1 child, so we collapse it (if near enough) and mark the extra depth
	TWO_CHILD_NODE,			//!< node has two children, if they are near enough in index space, collapse and mark extra depth
	USER_NODE_TYPE_START,	//!< where extensions node types start from
	MAX_TYPES = (1 << NODE_TYPE_BIT_SIZE)
};

#if defined( NEAR )
#undef NEAR
#endif
#if defined( FAR )
#undef FAR
#endif

static const int	LEFT	= 0x0;
static const int 	RIGHT	= 0x1;
static const int 	DOWN	= 0x0;
static const int 	UP		= 0x2;
static const int 	NEAR	= 0x0;
static const int 	FAR		= 0x4;

//! x: - Left = L	 + Right = R
//! y: + Up = U		 - Down = D
//! z: - Near  = N	 + Far = F
//! always that order, give you 8 combos, which are the consistent names of each node in an octree
enum class ChildName {
	LUN = LEFT | UP | NEAR,
	RUN = RIGHT | UP | NEAR,
	LDN = LEFT | DOWN | NEAR,
	RDN = RIGHT | DOWN | NEAR,
	LUF = LEFT | UP | FAR,
	RUF = RIGHT | UP | FAR,
	LDF = LEFT | DOWN | FAR,
	RDF = RIGHT | DOWN | FAR,
};

//! Node is a 32 bit uint varient, its represented as union of the actual payload with
//! a 4 bit type always present in each subtype
union Node {
	// this is the raw untyped bitfield (takes 32bit of space
	uint32_t 		type			: NODE_TYPE_BIT_SIZE; 		//!< actually NodeType enum or a descendant
	uint32_t		payload			: NODE_PAYLOAD_BIT_SIZE; 	//!< payload is a 28 bit variable data
	// each specific type has its own type and payload specific bitfield
	struct NodePayload {
		uint32_t					: NODE_TYPE_BIT_SIZE;
		uint32_t	nodeTileIndex 	: NODE_INDEX_BIT_SIZE;	//!< node tile of this nodes children
	} node;
	struct OnlyChildNodePayload {
		uint32_t					: NODE_TYPE_BIT_SIZE;
		uint32_t	nodeCode		: 3; //!< where node resides
		uint32_t	nodeTileIndex 	: NODE_ONLY_CHILD_INDEX_BIT_SIZE;	//!< node tile of this nodes children
	} onlyChildNode;
	struct TwoChildNodePayload {
		uint32_t					: NODE_TYPE_BIT_SIZE;
		uint32_t	nodeATileIndex 	: NODE_TWO_CHILD_INDEX_BIT_SIZE;	//!< relative nodeA address
		uint32_t	nodeACode		: 3; //!< where nodeA resides
		uint32_t	nodeBTileIndex 	: NODE_TWO_CHILD_INDEX_BIT_SIZE;	//!< relative nodeA address
		uint32_t	nodeBCode		: 3; //!< where nodeB resides
	} twoChildNode;
	struct LeafPayload {
		uint32_t					: NODE_TYPE_BIT_SIZE;
		uint32_t	tmpFlag			: 1;						//!< indicates an extension can re-generate at will
		uint32_t	leafIndex 		: NODE_LEAF_INDEX_BIT_SIZE;	//!< leaf this node uses
	} leaf;
	struct ConstantLeaf {
		uint32_t					: NODE_TYPE_BIT_SIZE;
		uint32_t	tmpFlag			: 1;
		uint32_t	leafIndex 		: NODE_LEAF_INDEX_BIT_SIZE;	//!< leaf its 8 children will use
	} constantLeaf;
	struct PackedBinaryLeaf {
		uint32_t					: NODE_TYPE_BIT_SIZE;
		uint32_t	occupancy		: 8;	//!< 8 bits one for each octant
		uint32_t	trueLeafIndex	: NODE_PACKED_BINARY_BIT_SIZE;	//!< index of brick ( only the first 1024 leaf are packable) to replicate with true occupancy bit
		uint32_t	falseLeafIndex	: NODE_PACKED_BINARY_BIT_SIZE;	//!< index of leaf ( only the first 1024 leaf are packable) to replicate with false occupancy bit
	} packedBinaryLeaf;
	struct Empty {
		uint32_t					: NODE_TYPE_BIT_SIZE;
		uint32_t	nothing			: NODE_PAYLOAD_BIT_SIZE;	//!< nada
	} empty;
};

static_assert( sizeof(Node) == (NODE_TYPE_BIT_SIZE + NODE_PAYLOAD_BIT_SIZE)/8, "Eeek vox::Node not 4 bytes in size" );

struct NodeTile { 
	Node nodes[8];
};

enum class BrickSize : uint32_t {
	TWO_CUBED,
	EIGHT_CUBED,
	SIXTEEN_CUBED,
	SIXTYFOUR_CUBED
};

//! Bricks are the smallest element in the voxtree, leaves of the tree
//! Along with the graphical data to render the tree, they also contain
//! the type of block and other data for gameplay systems.
//! uwv coordinate 9 bit per coordinate implicit 0 bit so 10 bit total 1024^3 address space
struct Brick {
	uint16_t	type;				//!< game specific type data (material etc.)

	uint16_t	splitable 	: 1;	//!< can brick be split into 8 smaller version of itself
	uint16_t	userData 	: 14;	//!< extra game data specific to each type.

	BrickSize	size 		: 2;	//!< size of voxel brick 2^3 to 64^3
	uint32_t	reflectX 	: 1;	//!< if voxel data reflected in x
	uint32_t	reflectY 	: 1;	//!< if voxel data reflected in y
	uint32_t	reflectZ 	: 1;	//!< if voxel data reflected in z
	uint32_t	gfxU 		: 9;	//!< brick gfx U coord
	uint32_t	gfxV 		: 9;	//!< brick gfx V coord
	uint32_t	gfxW 		: 9;	//!< brick gfx W coord
};
static_assert( sizeof(Brick) == 8, "Eeek vox::Brick not 8 bytes in size" );


enum class CULL_FUNC_RETURN {
	CULL 			= 0,
	CONTINUE,
	DEEP_ENOUGH		// HINT same as CONTINUE, hint that it doesn't need any more detailed data
};

template< typename Derived >
class Tree {
public:
	Derived* derived() { return static_cast<Derived*>(this); }
	const Derived* derived() const { return static_cast<const Derived*>(this); }

	class VisitHelper {
	public:
		friend class Tree;

		// get a node tile (i.e. the 8 children of a node )
		NodeTile& getNodeTile( const int _index );
		const NodeTile& getNodeTile( const int _index ) const;

		/// return the size of a node from its level
		Math::Vector3 getNodeSize( int _level ) const;

		/// return the volume of a node at a given level 
		float getNodeVolume( int _level ) const { Math::Vector3 v = getNodeSize( _level ); return v[0] * v[1] * v[2]; }

		/// returns a child AABB give its parent and the childs name
		Core::AABB	getChildBoundingBox( const ChildName _childName, const Core::AABB& _parent ) const;

		const Core::AABB& getRootBoundingBox() const { return parent.boundingBox; }
	private:
		VisitHelper( Tree& _parent );
		VisitHelper( const Tree& _parent );
		Tree& 		parent;
	};

	typedef std::function< void ( const VisitHelper& _helper ) > DescendConstFunc; 
	typedef std::function< void ( VisitHelper& _helper ) > DescendFunc; 

	Tree( const Core::AABB& _box );
	~Tree();

	// return true if node should be culled
	typedef std::function< CULL_FUNC_RETURN ( const Core::AABB& _aabb ) > NodeCullFunc; 

	typedef std::function< void ( const VisitHelper& _helper, const Node& _node, const Core::AABB& _aabb ) > LeafVisitConstFunc; 
	typedef std::function< bool ( VisitHelper& _helper, Node& _node, const Core::AABB& _aabb ) > LeafVisitFunc; 

	void visitLeaves( NodeCullFunc _cullFunc, LeafVisitConstFunc _leafFunc, bool _removeEmpty = true );
	void visitLeaves( NodeCullFunc _cullFunc, LeafVisitConstFunc _leafFunc, bool _removeEmpty = true ) const;

	void pack();

	/// Insert a point into the vox tree. If a point occupying the same space already exists
	/// it will be replaced with the leaf indicated
	/// \param _treeSpacePos position in tree space
	/// \param _leafIndex the leaf to be added
	/// \param _res points are always a single node, _res controls the point's volume 
	/// \return success or failure
	bool insertPoint( 	const Math::Vector3& _treeSpacePos,
						const uint32_t _leafIndex, 
						const float _res = DEFAULT_LEAF_RESOLUTION );

	/// does the node have a child tile index (i.e. getChildrenTileIndex is valid)
	static bool nodeHasChildren( const Node& _node );
	/// can the node be split (i.e. gain a child tile index )
	static bool nodeIsSplitable( const Node& _node );
	/// is the node empty or leaf or does it have descendants (packed or via tile index pointers)
	static bool nodeHasDescendants( const Node& _node );
	/// for nodes with children returns the child tile index, some splits can have two tile indices
	static uint32_t nodeGetChildrenTileIndex( const Node& _node, const uint32_t _thisIndex, uint32_t& _outSecondChildTile );

protected:
	struct TileNodeAndAABB {
		TileNodeAndAABB() {}
		TileNodeAndAABB( uint32_t _index, const Core::AABB _aabb ) :
			index(_index), aabb(_aabb) {}
		uint32_t index;
		Core::AABB aabb;
	};

	void freeNodeDescendants( Node& _node );
	/// splitNode may cause a resize, if so will update the _node pointer passed in, however other node pointers will be invalidated!
	uint32_t splitNode( Node** _node );
	void setNodeToLeaf( Node& _node, uint32_t _leafIndex );
	bool packNode( Node& _node );
	void packNodeAndDescendants( Node& _node );

	/// get the tile index the node lives in
	uint32_t getTileIndex( const Node& _node) const;

	typedef Core::FreeList< NodeTile > 	NodeTileContainer;

	NodeTileContainer				nodeTiles;

	const Core::AABB				boundingBox;
};

struct TreeBase : public Tree<TreeBase> {
	TreeBase( const Core::AABB& _box ) : Tree(_box) {}

	bool visit( VisitHelper& _helper, 
				Node& _node, 
				const Core::AABB& _aabb,
				NodeCullFunc _cullFunc, 
				LeafVisitConstFunc _leafFunc ) { return false; };

	void split( Node** _node, size_t _tileIndex ) {};
	static bool nodeHasChildren( const Node& _node ) { return false; }
	static bool nodeIsSplitable( const Node& _node ) { return false; }
	static bool nodeHasDescendants( const Node& _node ) { return false; }
	static uint32_t nodeGetChildrenTileIndex( const Node& _node, const uint32_t _thisIndex, uint32_t& _outSecondChildTile ) { 
		CORE_ASSERT(false); return INVALID_INDEX; 
	}
};

}

#include "voxtree.inl"

#endif