#include "core/core.h"
#include "voxtree.h"
#include "proceduraltextureutils.h"
#include "procvoxtree.h"
namespace Vox {
ProcVoxTree::ProcVoxTree( const Core::AABB& _box ) : Tree(_box) {
	for( int i = 0;i < 8; ++i ) {
		nodeTiles.get(0).nodes[i].type = ProcVoxNodeType::PROCEDURAL_NODE;
	}
}

bool ProcVoxTree::visit( 	VisitHelper& _helper, 
							Node& _node, 
							const Core::AABB& _aabb, 
							NodeCullFunc _cullFunc, 
							LeafVisitConstFunc _leafFunc ) {
	switch( _node.type ) {
		case PROCEDURAL_NODE: {
			return generate( _helper, _aabb, _cullFunc, _leafFunc );
		}
		break;
		default:
			CORE_ASSERT( false );
		break;
	}
}

bool ProcVoxTree::generate( 	VisitHelper& _helper, 
								const Core::AABB& _aabb, 
								NodeCullFunc _cullFunc, 
								LeafVisitConstFunc _leafFunc ) {
	std::stack<const Core::AABB> aabbStack;

	const float totalSizeRecip = 1.f/_helper.getRootBoundingBox().getHalfLength()[0];
	aabbStack.push( _aabb );

	while( !aabbStack.empty() ) {
		const Core::AABB aabb = aabbStack.top(); aabbStack.pop();
		for( int i = 0; i <8; ++i ) {
			const auto boundingBox = _helper.getChildBoundingBox( (ChildName)i, aabb );
			CULL_FUNC_RETURN cullRet = _cullFunc( boundingBox );
			if( cullRet == CULL_FUNC_RETURN::CULL ) {
				continue;			
			}

			float val = FBm( boundingBox.getBoxCenter() * totalSizeRecip , 2.3f, 2 );
			if( val > 0.999f ) {
				if( cullRet == CULL_FUNC_RETURN::CONTINUE && boundingBox.getHalfLength()[0] > 10.f ) {
					aabbStack.push( boundingBox );
				} else {
					Node node;
					node.type = NodeType::LEAF;
					node.leaf.leafIndex = 1; // TODO
					_leafFunc( _helper, node, boundingBox );
				}
			}
		}
	}
	return false;
}

} // end namespace Vox