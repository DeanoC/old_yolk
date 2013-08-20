#include "core/core.h"
#include "scene/renderer.h"
#include "voxtree.h"
#include "proceduraltextureutils.h"
#include "procvoxtree.h"

#if defined( USE_AMP )
#include "amp.h"
#endif

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

	const Math::Vector3 totalTranslate = _helper.getRootBoundingBox().getHalfLength();
	const float totalSizeRecip = 1.f/_helper.getRootBoundingBox().getHalfLength()[0];
	aabbStack.push( _aabb );

	while( !aabbStack.empty() ) {
		const Core::AABB aabb = aabbStack.top(); aabbStack.pop();
		for( int i = 0; i < 8; ++i ) {
			const auto boundingBox = _helper.getChildBoundingBox( (ChildName)i, aabb );
			CULL_FUNC_RETURN cullRet = _cullFunc( boundingBox );
			if( cullRet == CULL_FUNC_RETURN::CULL ) {
				continue;
			}

			float val = FBm( (totalTranslate + boundingBox.getBoxCenter() )* totalSizeRecip , 2.3f, 5 );
			if( val > 0.999f ) {
				if( cullRet == CULL_FUNC_RETURN::CONTINUE && 
						boundingBox.getHalfLength()[0] > 1.f ) {
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

void ProcVoxTree::ampTest( Scene::Renderer* _renderer ) {
#if defined( USE_AMP )
	concurrency::accelerator_view ampAcc = _renderer->getAMPAcceleratorView();

	int v[11] = {'G', 'd', 'k', 'k', 'n', 31, 'v', 'n', 'q', 'k', 'c'};
	concurrency::array_view<int,1> av( 11, v ); 

	concurrency::parallel_for_each( av.extent, 
		[=](concurrency::index<1> idx) restrict(amp) {
			av[idx] += 1;
		} 
	);	

	av.synchronize();

	for( int i=0;i < 11; ++i ) {
		LOG(INFO) << (char)v[i];
	}
	CORE_ASSERT( v[0] != 'G' );	
#endif
}

} // end namespace Vox