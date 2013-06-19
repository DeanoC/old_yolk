#include "core/core.h"
#include "voxtree.h"

#include "voxtreerenderable.h"

namespace Vox {

TreeRenderable::TreeRenderable( Tree& _vtree ) : 
	Renderable( CORE_NEW Core::TransformNode( worldMatrix ) ),
	tree( _vtree ) {
}

TreeRenderable::~TreeRenderable() {
	CORE_DELETE( transformNode );
}

void TreeRenderable::getRenderablesOfType( uint32_t _type, std::vector<Scene::Renderable*>& _out ) const {
}

void TreeRenderable::getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Scene::Renderable*>& _out ) const {

	tree.visit( [&]( const Tree::VisitHelper& _helper ) {
		struct TileNodeAndAABB {
			TileNodeAndAABB() {}
			TileNodeAndAABB( uint32_t _index, const Core::AABB _aabb ) :
				index(_index), aabb(_aabb) {}
			uint32_t index;
			Core::AABB aabb;
		};
		std::stack<TileNodeAndAABB> tileStack;

		if( _frustum.cullAABB( _helper.getRootBoundingBox() ) == Core::Frustum::CULL_RESULT::OUTSIDE )
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
				if( _frustum.cullAABB( bb ) == Core::Frustum::CULL_RESULT::OUTSIDE ) {
					continue;
				}

				switch( node.type ) {
					case NodeType::NODE: // push its children onto stack
						tileStack.push( TileNodeAndAABB( node.node.nodeTileIndex, bb ));
						break;
					case NodeType::ONLY_CHILD_NODE: {
						// another level to check
						const auto cbb = _helper.getChildBoundingBox( (ChildName)node.onlyChildNode.nodeCode, bb );
						if( _frustum.cullAABB( cbb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) { 
							tileStack.push( TileNodeAndAABB( node.onlyChildNode.nodeTileIndex, cbb ));
						}
						break;
					}
					case NodeType::TWO_CHILD_NODE: {
						// two nodes of a level down to check
						const auto abb = _helper.getChildBoundingBox( (ChildName)node.twoChildNode.nodeACode, bb );
						if( _frustum.cullAABB( abb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) {
							tileStack.push( TileNodeAndAABB( item.index + node.twoChildNode.nodeATileIndex, abb ));
						}
						const auto bbb = _helper.getChildBoundingBox( (ChildName)node.twoChildNode.nodeBCode, bb );
						if( _frustum.cullAABB( bbb ) != Core::Frustum::CULL_RESULT::OUTSIDE ) {
							tileStack.push( TileNodeAndAABB( item.index + node.twoChildNode.nodeBTileIndex, bbb ));
						}
						break;
					}
					case NodeType::LEAF:
					case NodeType::CONSTANT_LEAF:
					case NodeType::PACKED_BINARY_LEAF: {
						// render
						break;
					}
					default:
						break;
				}

			}
		}
	});

}

}