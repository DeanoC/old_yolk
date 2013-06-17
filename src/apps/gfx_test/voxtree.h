#pragma once
#ifndef YOLK_VOXTREE_H_
#define YOLK_VOXTREE_H_ 1

#include "core/aabb.h"

namespace Vox {

//! the normal size a  point like brick will be represented in the voxtree
static const float DEFAULT_BRICK_RESOLUTION		= 1.0f;

static const int NODE_TYPE_BIT_SIZE 			= 4;
static const int NODE_PAYLOAD_BIT_SIZE 			= 28;
static const int PACKED_BINARY_LUT_BIT_SIZE 	= 10;
static const int NODE_LEAF_INDEX_BIT_SIZE	 	= 27;
static const uint32_t INVALID_INDEX				= ~0;

// actually we only use 4 bits! but C++ doesn't have a nibble type which it makes it better to use the same base type
// of the bitfield you are sticking it in
enum class NodeType : uint32_t {
	EMPTY = 0,				//!< nada, technically redudent as PACKED_BINARY_LEAF could be used
	NODE,					//!< standard node, with an index to a children node tile
	LEAF,					//!< leaf node, has an index to a brick
	CONSTANT_LEAF,			//!< all children use the same brick
	PACKED_BINARY_LEAF,		//!< children only use 2 bricks, so are packed here without a nodetile

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
	struct Empty {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	nothing			: NODE_PAYLOAD_BIT_SIZE;	//!< nada
	} empty;
	struct NodePayload {
		NodeType					: NODE_TYPE_BIT_SIZE;
		uint32_t	nodeTileIndex 	: NODE_PAYLOAD_BIT_SIZE;	//!< node tile of this nodes children
	} node;
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
		uint32_t	trueBrickIndex	: PACKED_BINARY_LUT_BIT_SIZE;	//!< index in LUT to get brick to replicate with true occupancy bit
		uint32_t	falseBrickIndex	: PACKED_BINARY_LUT_BIT_SIZE;	//!< index in brick LUT for false occupancy, 0 == empty
	} packedBinaryLeaf;

	/// does this node have a child tile index (i.e. getChildrenTileIndex is valid)
	bool hasChildren() const;
	/// can the node be split (i.e. gain a child tile index )
	bool isSplitable() const;
	/// is the a empty or brick leaf or have descendants (packed or via tile index pointers)
	bool hasDescendants() const;

	/// for nodes with children returns the child tile index 
	uint32_t getChildrenTileIndex() const;
};

static_assert( sizeof(Node) == (NODE_TYPE_BIT_SIZE + NODE_PAYLOAD_BIT_SIZE)/8, "Eeek vox::Node not 4 bytes in size" );

// NOT MT safe in general
// keeps freelist always big enough for all data
// TODO make this std-a-like interface 
template< typename TYPE, typename INDEX_TYPE = uintptr_t >
class FreeList {
public:
	typedef typename std::vector< TYPE >::value_type value_type;
	typedef typename std::vector< TYPE >::size_type size_type;
	typedef typename std::vector< TYPE >::difference_type difference_type;
	typedef typename std::vector< TYPE >::pointer pointer;
	typedef typename std::vector< TYPE >::const_pointer const_pointer;
	typedef typename std::vector< TYPE >::reference reference;
	typedef typename std::vector< TYPE >::const_reference const_reference;	

	FreeList() : capacity(0), current(0), currentFree(0) {}

	explicit FreeList( size_type _count ) : data( _count), freelist( _count), currentFree( _count ) {
				for( size_type i = 0; i < _count; ++i ) { 
					freelist[i] = i; 
				}

			}

	INDEX_TYPE push( const value_type& _val ) {
		INDEX_TYPE index = alloc();
		data[ index ] = _val;
		return index;
	}

	INDEX_TYPE alloc() {
		INDEX_TYPE index;
		if( freelist.empty() ) {
			index = data.size();
			resize( index + 1 );
		} else {
			CORE_ASSERT( currentFree != 0 );
			index = freelist[--currentFree];
		}
		return index;
	}

	void resize( const size_type _count ) {
		data.resize( _count );
		freelist.resize( _count );
	}

	bool empty() const { return currentFree == freelist.size(); }

	void erase( const INDEX_TYPE _index ) {
		CORE_ASSERT( _index < freelist.size() );
		freelist[ currentFree++ ] = _index;
	}

	TYPE& get( const INDEX_TYPE _index ) { return data[_index]; }
	const TYPE& get( const INDEX_TYPE _index ) const { return data[_index]; }

private:
	std::vector< TYPE >				data;
	std::vector< INDEX_TYPE > 		freelist;
	INDEX_TYPE						currentFree;
};

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

	uint16_t	packable 	: 1;	//!< whether this is a packable type (only 1024 types can be)
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
	uint32_t splitNode( Node& _node );
	void setNodeToBrick( Node& _node, uint32_t _brickIndex );
	void packNode( Node& _node );

	typedef FreeList< NodeTile > NodeTileContainer;
	typedef FreeList< Brick >	BrickContainer;

	NodeTileContainer				nodeTiles;
	BrickContainer 					bricks;
	Node 							rootNode;

	uint32_t						packedBinaryLUT[ 1 << PACKED_BINARY_LUT_BIT_SIZE ];

	const Core::AABB				boundingBox;
};

}

#endif