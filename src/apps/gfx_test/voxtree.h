#pragma once
#ifndef YOLK_VOXTREE_H_
#define YOLK_VOXTREE_H_ 1

#include "core/aabb.h"
#include "freelist.h" // TODO move to Core

namespace Vox {

//! the normal size a  point like brick will be represented in the voxtree
static const float DEFAULT_BRICK_RESOLUTION		= 1.0f;

static const int NODE_TYPE_BIT_SIZE 			= 4;
static const int NODE_PAYLOAD_BIT_SIZE 			= 28;
static const int PACKED_BINARY_BIT_SIZE 		= 10;
static const int NODE_LEAF_INDEX_BIT_SIZE	 	= 27;
static const int NODE_INDEX_BIT_SIZE 			= 28;
static const int NODE_ONLY_CHILD_INDEX_BIT_SIZE = 25;
static const int NODE_TWO_CHILD_INDEX_BIT_SIZE = 11;

static const uint32_t INVALID_INDEX				= ~0;

// actually we only use 4 bits! but C++ doesn't have a nibble type which it makes it better to use the same base type
// of the bitfield you are sticking it in
enum class NodeType : uint32_t {
	EMPTY = 0,				//!< nada, technically redudent as CONSTNAT_LEAF could be used
	NODE,					//!< standard node, with an index to a children node tile
	LEAF,					//!< leaf node, has an index to a brick
	CONSTANT_LEAF,			//!< all children use the same brick
	PACKED_BINARY_LEAF,		//!< children only use 2 bricks, so are packed here without a nodetile
	ONLY_CHILD_NODE,		//!< the node only has 1 child, so we collapse it (if near enough) and mark the extra depth
	TWO_CHILD_NODE,			//!< node has two children, if they are near enough in index space, collapse and mark extra depth
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
//! always that order, give you 8 combos, which are the consitant names of each node in an octree
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
//! brick/leaf splitable is usually inserted into the node tree for cache reason, however sometimes
//! we are out of bits so it will need to be recovered from the brick itself 
union Node {
	// this is the raw untyped bitfield (takes 32bit of space
	NodeType 		type			: NODE_TYPE_BIT_SIZE;
	uint32_t		payload			: NODE_PAYLOAD_BIT_SIZE; //< payload is a 28 bit variable data
	// each specific type has its own type and payload specific bitfield
	struct NodePayload {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	nodeTileIndex 	: NODE_INDEX_BIT_SIZE;	//!< node tile of this nodes children
	} node;
	struct OnlyChildNodePayload {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	nodeCode		: 3; //!< where node resides
		uint32_t	nodeTileIndex 	: NODE_ONLY_CHILD_INDEX_BIT_SIZE;	//!< node tile of this nodes children
	} onlyChildNode;
	struct TwoChildNodePayload {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	nodeATileIndex 	: NODE_TWO_CHILD_INDEX_BIT_SIZE;	//!< relative nodeA address
		uint32_t	nodeACode		: 3; //!< where nodeA resides
		uint32_t	nodeBTileIndex 	: NODE_TWO_CHILD_INDEX_BIT_SIZE;	//!< relative nodeA address
		uint32_t	nodeBCode		: 3; //!< where nodeB resides
	} twoChildNode;
	struct LeafPayload {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	splitable 		: 1;	//!< can brick be split into 8 smaller version of itself
		uint32_t	brickIndex 		: NODE_LEAF_INDEX_BIT_SIZE;	//!< brick this node uses
	} leaf;
	struct ConstantLeaf {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	splitable 		: 1;	//!< can each CHILD brick be split into 8 smaller versions
		uint32_t	brickIndex 		: NODE_LEAF_INDEX_BIT_SIZE;	//!< brick its 8 children will use
	} constantLeaf;
	struct PackedBinaryLeaf {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	occupancy		: 8;	//!< 8 bits one for each octant
		uint32_t	trueBrickIndex	: PACKED_BINARY_BIT_SIZE;	//!< index of brick ( only the first 1024 bricks are packable) to replicate with true occupancy bit
		uint32_t	falseBrickIndex	: PACKED_BINARY_BIT_SIZE;	//!< index of brick ( only the first 1024 bricks are packable) to replicate with false occupancy bit
	} packedBinaryLeaf;
	struct Empty {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	nothing			: NODE_PAYLOAD_BIT_SIZE;	//!< nada
	} empty;

	/// does this node have a child tile index (i.e. getChildrenTileIndex is valid)
	bool hasChildren() const;
	/// can the node be split (i.e. gain a child tile index )
	bool isSplitable() const;
	/// is the a empty or brick leaf or have descendants (packed or via tile index pointers)
	bool hasDescendants() const;

	/// for nodes with children returns the child tile index, some splits can have two tile indices
	uint32_t getChildrenTileIndex( const uint32_t _thisIndex, uint32_t& _outSecondChildTile ) const;
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

//! Bricks are the smallist element in the voxtree, leaves of the tree
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

/// tree space == coordinate system based on tree origin etc.
class Tree {
public:
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

	void visit( DescendConstFunc _func ) const;
	void visit( DescendFunc _func );

	void pack();

	uint32_t allocateBrick( Brick** outBrick );

	/// Insert a point into the vox tree. If a point occupying the same space already exists
	/// it will be replaced with the brick indicated
	/// \param _treeSpacePos position in tree space
	/// \param _brickIndex the brick to be added
	/// \param _res points are always a single node, _res controls the point's volume 
	/// \return success or failuer
	bool insertPoint( 	const Math::Vector3& _treeSpacePos,
						const uint32_t _brickIndex, 
						const float _res = DEFAULT_BRICK_RESOLUTION );

private:
	void freeNodeDescendants( Node& _node );
	/// splitNode may cause a resize, if so will update the _node pointer passed in, however other node pointers will be invalidated!
	uint32_t splitNode( Node** _node );
	void setNodeToBrick( Node& _node, uint32_t _brickIndex );
	void packNode( Node& _node );
	void packNodeAndDescendants( Node& _node );


	/// get the tile index the node lives in
	uint32_t getTileIndex( Node& _node) const;

	typedef Core::FreeList< NodeTile > NodeTileContainer;
	typedef Core::FreeList< Brick >	BrickContainer;

	NodeTileContainer				nodeTiles;
	BrickContainer 					bricks;

	const Core::AABB				boundingBox;
};

}

#endif