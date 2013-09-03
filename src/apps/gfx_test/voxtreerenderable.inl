#include "core/core.h"
#include "core/debug_render.h"
#include "scene/mesh.h"
#include "voxtree.h"

#include "voxtreerenderable.h"

namespace Vox {

template< typename TreeType >
class WallRenderer;

// proxy for instance rendering etch brick is a renderable to just passes the call back to Wall with its number
template< typename TreeType >
class BrickRenderable : public Scene::Renderable {
public:
	BrickRenderable() {};

	void init( WallRenderer<TreeType>* _owner, uint32_t _number ) {
		owner = _owner;
		number = _number;
	}

	static const uint32_t BRICK_TYPE = Core::GenerateID<'B','R','I','K'>::value;

	void render( Scene::RenderContext* _context, const Scene::Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix  ) const override;

	// think i need to split raw renderables from gatherable renderables but for now...
	void getRenderablesOfType( uint32_t _type, std::vector<Renderable*>& _out ) const override {};
	void getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Renderable*>& _out ) const override {};

	const Math::Matrix4x4& getRenderMatrix() const override;
//////////
	WallRenderer<TreeType>*	owner;
	uint32_t		number;
};

struct BrickInstanceData {
	BrickInstanceData() : brickIndex(~0) {}
	BrickInstanceData( const uint32_t _brickIndex, const Core::AABB& _aabb ) : brickIndex( _brickIndex ), aabb( _aabb ) {}

	uint32_t brickIndex;
	Core::AABB aabb;		//todo compress aabb
};

template< typename TreeType >
class WallRenderer {
public:
	friend class BrickRenderable<TreeType>;

	WallRenderer( const TreeType& _tree ) : tree( _tree ) {
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
	std::vector< BrickRenderable<TreeType> >		brickRenderables;
	std::vector< BrickInstanceData >	brickInstanceData;
	std::vector< Math::Matrix4x4>		brickMatrix;

	const TreeType& 						tree;
};

template< typename TreeType >
TreeRenderable<TreeType>::TreeRenderable( Core::TransformNode* _transform, const TreeType& _tree ) : 
	Renderable( _transform ),
	tree( _tree ),
	wallRenderer( CORE_NEW WallRenderer<TreeType>(_tree ) ) {
}

template< typename TreeType >
TreeRenderable<TreeType>::~TreeRenderable() {
	CORE_DELETE( wallRenderer );
}

template< typename TreeType >
void TreeRenderable<TreeType>::getRenderablesOfType( uint32_t _type, std::vector<Scene::Renderable*>& _out ) const {
}

template< typename TreeType >
void TreeRenderable<TreeType>::getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Scene::Renderable*>& _out ) const {

	// for culling to work we want to move the frustum into local tree space
	const Core::Frustum localFrustum( getTransformNode()->getRenderMatrix() * _frustum.matrix );

	wallRenderer->beginInstanceGather();

	auto& wallRender = *wallRenderer;

	tree.visitLeaves( 
		// cull function
		[&_frustum, &wallRender]( const Core::AABB& _aabb ) -> Vox::CULL_FUNC_RETURN {
			if( _frustum.cullAABB( _aabb ) == Core::Frustum::CULL_RESULT::OUTSIDE ) {
				return Vox::CULL_FUNC_RETURN::CULL;
			} else {
				Math::Vector3 minEx = Math::TransformAndProject( _aabb.getMinExtent(), _frustum.matrix );
				Math::Vector3 maxEx = Math::TransformAndProject( _aabb.getMaxExtent(), _frustum.matrix );
				if( Math::LengthSquared(maxEx - minEx) < (0.15f*0.15f) ) {
					return Vox::CULL_FUNC_RETURN::DEEP_ENOUGH;
				} else {
					return Vox::CULL_FUNC_RETURN::CONTINUE;
				}
			}
		}, 
		// leaf visit function
		[&wallRender]( const TreeType::VisitHelper& _helper, const Vox::Node& _node, const Core::AABB& _aabb ) -> void {
			switch( _node.type ) {
				case NodeType::LEAF:
					wallRender.addInstance( BrickInstanceData( _node.leaf.leafIndex, _aabb ) );
					break;
				case NodeType::PACKED_BINARY_LEAF: {
					for( int i = 0; i < 8; i++ ) {
						bool posiOcc = !!(_node.packedBinaryLeaf.occupancy & (1 << i));
						if(  posiOcc == true ||
							( posiOcc == false && _node.packedBinaryLeaf.falseLeafIndex != 0 ) ) {
							wallRender.addInstance( BrickInstanceData( 
													posiOcc ? _node.packedBinaryLeaf.trueLeafIndex : _node.packedBinaryLeaf.falseLeafIndex, 
													_helper.getChildBoundingBox( (ChildName) i, _aabb ) ) );
						}
					}
					break;
				}
			}
		}
	);
	wallRenderer->endInstanceGather( _out );

	Core::g_pDebugRender->varPrintf( 0, Math::Vector2(-1, 0.9f), "bricks visible %i", _out.size() );

}

template< typename TreeType >
void BrickRenderable<TreeType>::render( Scene::RenderContext* _context, const Scene::Pipeline* _pipeline, const Math::Matrix4x4 _renderMatrix  ) const {
	owner->brickMeshes[0]->render( _context, _pipeline, _renderMatrix );
}

template< typename TreeType >
const Math::Matrix4x4& BrickRenderable<TreeType>::getRenderMatrix() const {
	return owner->brickMatrix[number];
}


}