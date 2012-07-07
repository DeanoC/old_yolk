#include "meshimport.h"

// assimp include files. These three are usually needed.
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/cimport.h"
#include "meshmod/materialface.h"
#include "assimp.h"
	

namespace MeshImport {

AssImp::AssImp( const std::string& filename ) {
	// leave any processing to GO for now
	scene = aiImportFile( filename.c_str(), aiProcess_ConvertToLeftHanded );
	tempVertexIndices.reserve(10); // enough for a 10 index polygon
}

AssImp::~AssImp() {
	aiReleaseImport( scene );
}


MeshMod::ScenePtr AssImp::toMeshMod() {
	MeshMod::ScenePtr root = std::make_shared<MeshMod::Scene>();
	root->sceneNodes.push_back( std::make_shared<MeshMod::SceneNode>() );

	if( scene != NULL ) {
		Recurse( root->sceneNodes[0], scene->mRootNode );
	}
	return root;
}

void AssImp::Recurse( MeshMod::SceneNodePtr gonode, const aiNode* node ) {

	// node->mTransformation
	aiMatrix4x4 trans = node->mTransformation;
	Math::Matrix4x4 transform( &trans.Transpose().a1 );

	MeshMod::MeshPtr gomesh;
	for( unsigned int i=0;i < node->mNumMeshes; ++i ) {
		const aiMesh* mesh = scene->mMeshes[ node->mMeshes[i] ];

		// do we have a mesh with any data to convert
		if( mesh->HasPositions() == true && mesh->HasFaces() == true ) {
			// a mesh in Ass is a material in our, we pack multiple Ass meshes
			// in a single of GO mesh
			if( i == 0 ) {
				gomesh = MeshMod::MeshPtr( CORE_NEW MeshMod::Mesh(node->mName.data) );
				gonode->addObject( gomesh );
			}
				
			ConvertAiMeshToGo( i, mesh, gomesh );
		}
	}

	for( unsigned int i=0;i < node->mNumChildren; ++i ) {
		MeshMod::SceneNodePtr child = MeshMod::SceneNodePtr( CORE_NEW MeshMod::SceneNode() );
		gonode->addChild( child );
		Recurse( child, node->mChildren[i] );
	}
}

void AssImp::ConvertAiMeshToGo( uint32_t matIndex, const aiMesh* mesh, MeshMod::MeshPtr gomesh ) {
	using namespace MeshMod;

	// do we have a mesh with any data to convert
	if( mesh->HasPositions() == false || mesh->HasFaces() == false ) {
		return;
	}

	VertexElementsContainer& vertCon = gomesh->getVertexContainer();
	FaceElementsContainer& faceCon = gomesh->getFaceContainer();
	tempVertexIndices.resize( 0 );

	// where this aimesh (material starts in the vertex pool)
	VertexIndex firstVertexIndex = gomesh->getVertexContainer().size();
	MaterialIndex firstMaterialIndex = 0;//gomesh->getMaterialContainer().size();
	
	// place to stored our per face material index
	MaterialFaceElements* faceMatEle = faceCon.getOrAddElements<MaterialFaceElements>();
	NormalVertexElements* normalVertEle = mesh->HasNormals() ? vertCon.getOrAddElements<NormalVertexElements>() : 0 ;

	UVVertexElements* uvVertEle[ 4 ]; // ass imp supports upto 4 texture coords so far

	uvVertEle[0] = mesh->HasTextureCoords( 0 ) ? vertCon.getOrAddElements<UVVertexElements>( "uv0" ) : 0;
	uvVertEle[1] = mesh->HasTextureCoords( 1 ) ? vertCon.getOrAddElements<UVVertexElements>( "uv1" ) : 0 ;
	uvVertEle[2] = mesh->HasTextureCoords( 2 ) ? vertCon.getOrAddElements<UVVertexElements>( "uv2" ) : 0 ;
	uvVertEle[3] = mesh->HasTextureCoords( 3 ) ? vertCon.getOrAddElements<UVVertexElements>( "uv3" ) : 0 ;

	// add every positions
	for( unsigned int i=0;i < mesh->mNumVertices; ++i ) {
		gomesh->addPosition( mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z );
	}

	// go round every face add position indices
	for( unsigned int i=0; i < mesh->mNumFaces; ++i ) {
		const aiFace* face = &mesh->mFaces[i];
		tempVertexIndices.resize( 0 );
		for( unsigned int j=0; j < face->mNumIndices; ++j ) {
			tempVertexIndices.push_back( face->mIndices[j] );
		}

		FaceIndex faceIndex = gomesh->addPolygon( tempVertexIndices, firstVertexIndex );
		(*faceMatEle)[ faceIndex ] = matIndex + firstMaterialIndex;

		for( unsigned int j=0; j < face->mNumIndices; ++j ) {
			if( mesh->HasNormals() ) {
				(*normalVertEle)[ face->mIndices[j] ] = VertexData::Normal( mesh->mNormals[ face->mIndices[j] ].x, 
																			mesh->mNormals[ face->mIndices[j] ].y, 
																			mesh->mNormals[ face->mIndices[j] ].z );
			}
			for( int k=0;k < 4;++k ) {
				if( mesh->HasTextureCoords( k ) ) {
					// TODO support 3D tex coords?
					(*uvVertEle[k])[ face->mIndices[j] ] = VertexData::UV(	mesh->mTextureCoords[k][ face->mIndices[j] ].x, 
																			mesh->mTextureCoords[k][ face->mIndices[j] ].y );
				}
			}
		}
	}

}

}