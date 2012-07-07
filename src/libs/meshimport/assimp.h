/** \file assimp.h
   (c) 2012 Dean Calver
*/
#if !defined( GOIMPORT_ASSIMP_H_ )
#define GOIMPORT_ASSIMP_H_
#pragma once

struct aiNode;
struct aiMesh;
struct aiScene;

namespace MeshImport {
	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------
	DECLARE_EXCEPTION( AssImportFileError, A error occured loading a AssImp object );

	/**
	Short description.
	Detailed description
	*/
	class AssImp  : public ImportInterface {
	public:
		AssImp( const std::string& filename );
		virtual ~AssImp();

		virtual bool loadedOk() const override { return scene != NULL; }
		virtual MeshMod::ScenePtr toMeshMod() override;

	private:
		void Recurse( MeshMod::SceneNodePtr gonode, const aiNode* node );
		void ConvertAiMeshToGo( uint32_t index, const aiMesh* mesh, MeshMod::MeshPtr gomesh );

		const aiScene* scene;

		MeshMod::VertexIndexContainer tempVertexIndices; // some temp space (just enough to hold 1 polygon)
	};
}

#endif