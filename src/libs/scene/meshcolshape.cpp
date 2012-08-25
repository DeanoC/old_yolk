#include "scene.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "core/coreresources.h"
#include "dynamicsproperties.h"
#include "meshcolshape.h"

namespace Scene {
	MeshColShape::MeshColShape( std::shared_ptr<Core::BinPropertyResource> _props ) :
		properties( _props ),
		indexVertexArray( nullptr ),
		trimeshShape( nullptr ) 
	{
		using namespace Core;
		std::unordered_map<std::string, int> nameMap;

		for( auto i = 0; i < properties->getNumProperties(); ++i ) {
			const std::string& name = properties->getName(i);
			nameMap[name] = i;
		}

		CORE_ASSERT( nameMap.find( DYNAMICS_MESH_INDICES ) != nameMap.cend() );
		CORE_ASSERT( nameMap.find( DYNAMICS_MESH_VERTICES ) != nameMap.cend() );

		int* indices;
		int indexStride;
		if( properties->getType( nameMap[ DYNAMICS_MESH_INDICES ] ) == BinProperty::BPT_UINT16 ) {
			indexStride = 2;
			indices = (int*) properties->getAs<uint16_t>( nameMap[ DYNAMICS_MESH_INDICES ] );
		} else if( properties->getType( nameMap[ DYNAMICS_MESH_INDICES ] ) == BinProperty::BPT_UINT32 ) {
			indexStride = 4;
			indices = (int*) properties->getAs<uint32_t>( nameMap[ DYNAMICS_MESH_INDICES ] );
		} else {
			CORE_ASSERT( false && "invalid dynamics_indices type" );
		}

		CORE_ASSERT( properties->getType( nameMap[ DYNAMICS_MESH_VERTICES ] ) == BinProperty::BPT_FLOAT );
		float* vpnts = (float*)properties->getAs<float>( nameMap[ DYNAMICS_MESH_VERTICES ] );

	    indexVertexArray = CORE_NEW btTriangleIndexVertexArray(
	    		properties->getCount( nameMap[ DYNAMICS_MESH_INDICES ] )/3,
	            indices,
	            indexStride * 3,
	        	properties->getCount( nameMap[ DYNAMICS_MESH_VERTICES ] )/3, 
	            vpnts, 
	            sizeof(float)*3 );

		trimeshShape  = CORE_NEW btBvhTriangleMeshShape( indexVertexArray, true );
	}
	MeshColShape::~MeshColShape() {
		CORE_DELETE indexVertexArray;
		CORE_DELETE trimeshShape;
	}
	btCollisionShape* MeshColShape::getBTCollisionShape() const { 
		return static_cast<btCollisionShape*>(trimeshShape); 
	}

}