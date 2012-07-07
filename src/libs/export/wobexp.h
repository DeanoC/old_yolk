/** \file out_wob.h
	Outputs a GO mesh into a wob file
   (c) 2005 Dean Calver
 */

#if !defined( SHORT_OUT_WOB_H_ )
#define SHORT_OUT_WOB_H_

#include "meshmod/uvvertex.h"
#include "meshmod/materialparameter.h"
#include "meshmod/sceneobject.h"
#include "meshmod/scene.h"
#include "core/file_path.h"
#include "core/resource_dir.h"
#include <set>

namespace Export {
	class WobbyWriter
	{
	public:

		// false if there is no mesh to save (empty)
		bool Save( MeshMod::MeshPtr goMesh, const Core::FilePath pOutFilename );

		/// false if can't write out the meshes, writes out all meshes in the node tree
		bool Save( MeshMod::SceneNodePtr rootNode, Core::ResourceManifestEntryVector& manifest, const Core::FilePath pOutFilename );

	protected:

		void WriteMaterial( const unsigned int matNum, const unsigned int uiFlags, std::ostream& outStream  );
		uint32_t WriteScalarParams( const MeshMod::MaterialData::ParameterContainer& params, 
									const std::string& txtBaseName,
									std::ostringstream& parameterString,
									std::ostringstream& parameterDataString );
		uint32_t WriteRGBParams(	const MeshMod::MaterialData::ParameterContainer& params, 
									const std::string& txtBaseName,
									std::ostringstream& parameterString,
									std::ostringstream& parameterDataString );
		uint32_t WriteTextureParams(	const MeshMod::MaterialData::ParameterContainer& params, 
										const std::string& txtBaseName,
										std::ostringstream& parameterString,
										std::ostringstream& parameterDataString );

		//! any null elements gets written out
		void WriteVerticesAndIndices(	unsigned int matNum,
							const MeshMod::PositionVertexElements*						posEle,
							const MeshMod::NormalVertexElements*						normEle,
							const MeshMod::UVVertexElements*							uvEle,
							const MeshMod::BoneWeightsVertexElements*					boneEle,
							std::ostream& outStream );


		void ConvertLightParamsToMaterialParameters(	MeshMod::MaterialElementsContainer& matCon, 
														const MeshMod::LightParamsMaterialElements* lightElements );
		void CalcMaterialAABB( const unsigned int matNum, const MeshMod::PositionVertexElements* posEle, float outMin[3], float outMax[3] );
		void CalcMeshAAB( float outMin[3], float outMax[3] );

		typedef std::map< std::string, std::string> StringTable;

		StringTable m_stringTable;
		StringTable m_vertexDataTable;
		StringTable m_indexDataTable;
		StringTable m_parameterDataTable;

		MeshMod::MeshPtr	m_goMesh;

		boost::scoped_array< std::set<uint32_t> >		m_vertListArray;
		boost::scoped_array< std::vector<uint32_t> >	m_faceListArray;
	};
}

#endif
