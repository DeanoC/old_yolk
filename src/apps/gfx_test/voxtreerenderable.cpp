#include "core/core.h"
#include "core/debug_render.h"
#include "scene/mesh.h"
#include "voxtree.h"

#include "voxtreerenderable.h"

namespace Vox {

class WallRenderer;

// proxy for instance rendering etch brick is a renderable to just passes the call back to Wall with its number
class BrickRenderable : public Scene::Renderable {
public:
	BrickRenderable() {};

	void init( WallRenderer* _owner, uint32_t _number ) {
		owner = _owner;
		number = _number;
	}


	static const uint32_t BRICK_TYPE = Core::GenerateID<'B','R','I','K'>::value;

	void render( Scene::RenderContext* _context, const Scene::Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix  ) const override;

	// think i need to split raw renderables from gatherable renderables but for now...
	void getRenderablesOfType( uint32_t _type, std::vector<Renderable*>& _out ) const override {};
	void getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Renderable*>& _out ) const override {};

	const Math::Matrix4x4& getRenderMatrix() const override;

	WallRenderer*	owner;
	uint32_t		number;
};

struct BrickInstanceData {
	BrickInstanceData() : brickIndex(~0) {}
	BrickInstanceData( const uint32_t _brickIndex, const Core::AABB& _aabb ) : brickIndex( _brickIndex ), aabb( _aabb ) {}

	uint32_t brickIndex;
	Core::AABB aabb;		//todo compress aabb
};

class WallRenderer {
public:
	friend class BrickRenderable;

	WallRenderer( Tree& _vtree ) : tree( _vtree ) {
		brickMeshes.push_back( std::make_shared<Scene::Mesh>( "red_1x1cube"  ) );
	}

	virtual ~WallRenderer() {
	}

	void beginInstanceGather() {
		currentInstanceCount = 0;
	}
	void addInstance( const BrickInstanceData& _data ) {
		if( currentInstanceCount >= brickInstanceData.size() ) {
			brickRenderables.resize( currentInstanceCount+1 );
			brickInstanceData.resize( currentInstanceCount+1 );
			brickMatrix.resize( currentInstanceCount+1 );
			brickRenderables[ currentInstanceCount ].init( this, currentInstanceCount );
		}
		unsigned int index = currentInstanceCount++;
		brickInstanceData[ index ] = _data;
		brickMatrix[ index ] = Math::IdentityMatrix();
		Math::SetScale( brickMatrix[ index ], _data.aabb.getHalfLength() * 2 );
		Math::SetTranslation( brickMatrix[ index ], _data.aabb.getBoxCenter() );
	}

	void endInstanceGather( std::vector<Scene::Renderable*>& _out ) {
		for( unsigned int i = 0;i < currentInstanceCount; ++i ) {
			_out.push_back( &brickRenderables[i] );
		}
	}

protected:
	unsigned int currentInstanceCount;

	std::vector< Scene::MeshPtr >		brickMeshes;
	std::vector< BrickRenderable >		brickRenderables;
	std::vector< BrickInstanceData >	brickInstanceData;
	std::vector< Math::Matrix4x4>		brickMatrix;

	Tree 								tree;
	Math::Matrix4x4						worldMatrix;
};

TreeRenderable::TreeRenderable( Tree& _vtree ) : 
	Renderable( CORE_NEW Core::TransformNode( worldMatrix ) ),
	tree( _vtree ),
	wallRenderer( CORE_NEW WallRenderer(_vtree ) ) {
}

TreeRenderable::~TreeRenderable() {
	CORE_DELETE( wallRenderer );
	CORE_DELETE( transformNode );
}

void TreeRenderable::getRenderablesOfType( uint32_t _type, std::vector<Scene::Renderable*>& _out ) const {
}

void TreeRenderable::getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Scene::Renderable*>& _out ) const {

	// for culling to work we want to move the frstum into local tree space
	const Core::Frustum localFrustum( getTransformNode()->getRenderMatrix() * _frustum.matrix );

	wallRenderer->beginInstanceGather();

	WallRenderer& wallRender = *wallRenderer;
	tree.descend( [&localFrustum, &wallRender ]( const Tree::VisitHelper& _helper ) {
		struct TileNodeAndAABB {
			TileNodeAndAABB() {}
			TileNodeAndAABB( uint32_t _index, const Core::AABB _aabb ) :
				index(_index), aabb(_aabb) {}
			uint32_t index;
			Core::AABB aabb;
		};
		std::stack<TileNodeAndAABB> tileStack;

		if( localFrustum.cullAABB( _helper.getRootBoundingBox() ) == Core::Frustum::CULL_RESULT::OUTSIDE )
			return;

		TileNodeAndAABB rootItem( _helper.getNodeTile(0).nodes[0].node.nodeTileIndex, _helper.getRootBoundingBox() );
		tileStack.push( rootItem );

		while( tileStack.empty() == false ) {
			auto item = tileStack.top(); tileStack.pop();
			CORE_ASSERT( item.index != INVALID_INDEX );
			const NodeTile& nodeTile = _helper.getNodeTile( item.index );
			// check nodes children AABB
			for(int i = 0;i < 8; ++i ) {
				const Node& node = nodeTile.nodes[i];
				// early out for empties
				if( node.type == NodeType::EMPTY ) {
					continue;
				}
				// is node visible?
				const auto bb = _helper.getChildBoundingBox( (ChildName)i, item.aabb );
				if( localFrustum.cullAABB( bb ) == Core::Frustum::CULL_RESULT::OUTSIDE ) {
					continue;
				}

				switch( node.type ) {
					case NodeType::NODE: // push its children onto stack
						tileStack.push( TileNodeAndAABB( node.node.nodeTileIndex, bb ));
						break;
					case NodeType::ONLY_CHILD_NODE: {
						// another level to check
						const auto cbb = _helper.getChildBoundingBox( (ChildName)node.onlyChildNode.nodeCode, bb );
						if( localFrustum.cullAABB( cbb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) { 
							tileStack.push( TileNodeAndAABB( node.onlyChildNode.nodeTileIndex, cbb ));
						}
						break;
					}
					case NodeType::TWO_CHILD_NODE: {
						// two nodes of a level down to check
						const auto abb = _helper.getChildBoundingBox( (ChildName)node.twoChildNode.nodeACode, bb );
						if( localFrustum.cullAABB( abb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) {
							tileStack.push( TileNodeAndAABB( item.index + node.twoChildNode.nodeATileIndex, abb ));
						}
						const auto bbb = _helper.getChildBoundingBox( (ChildName)node.twoChildNode.nodeBCode, bb );
						if( localFrustum.cullAABB( bbb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) {
							tileStack.push( TileNodeAndAABB( item.index + node.twoChildNode.nodeBTileIndex, bbb ));
						}
						break;
					}
					case NodeType::LEAF:
						wallRender.addInstance( BrickInstanceData( node.leaf.brickIndex, bb ) );
						break;
					case NodeType::CONSTANT_LEAF: {
						wallRender.addInstance( 
							BrickInstanceData( node.constantLeaf.brickIndex, bb ) );
						break;
					}
					case NodeType::PACKED_BINARY_LEAF: {
						for( int i = 0; i < 8; i++ ) {
							bool posiOcc = !!(node.packedBinaryLeaf.occupancy & (1 << i));
							if(  posiOcc == true ||
								( posiOcc == false && node.packedBinaryLeaf.falseBrickIndex != 0 ) ) {
								wallRender.addInstance( BrickInstanceData( 
														posiOcc ? node.packedBinaryLeaf.trueBrickIndex : node.packedBinaryLeaf.falseBrickIndex, 
														_helper.getChildBoundingBox( (ChildName) i, bb ) ) );
							}
						}
						break;
					}
					default:
						break;
				}

			}
		}
	});

	wallRenderer->endInstanceGather( _out );

	Core::g_pDebugRender->varPrintf( 0, Math::Vector2(-1, 0.9f), "bricks visible %i", _out.size() );

}

void BrickRenderable::render( Scene::RenderContext* _context, const Scene::Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix  ) const {
	owner->brickMeshes[0]->render( _context, _pipeline, _renderMatrix );
}
const Math::Matrix4x4& BrickRenderable::getRenderMatrix() const {
	return owner->brickMatrix[number];
}


}