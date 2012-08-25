#include "export.h"

#include "core/coreresources.h"
#include "core/file_path.h"
#include "scene/wobfile.h"
#include "meshmod/materialparameter.h"
#include "meshmod/materialvertexbindings.h"
#include "meshmod/scalarmaterialparameter.h"
#include "meshmod/rgbmaterialparameter.h"
#include "meshmod/texturematerialparameter.h"
#include "meshmod/materialface.h"
#include "meshops/basicmeshops.h"

#include "wobexp.h"

#define isnan(x) ((x) != (x))

namespace Export {

uint32_t WobbyWriter::WriteScalarParams(	const MeshMod::MaterialData::ParameterContainer& params, 
											const std::string& txtBaseName,
											std::ostringstream& parameterString,
											std::ostringstream& parameterDataString)
{
	using namespace MeshMod;
	using namespace Scene;

	std::vector< const MaterialData::FloatScalarElements* > fSPContainer;
	params.getAllElements( fSPContainer );

	for( unsigned int i = 0; i < fSPContainer.size(); ++i ) {
		using namespace MaterialData;
		const FloatScalarElements* pParam = fSPContainer[i];
		std::string paramBaseName = pParam->subName;
		// the usual case is one element > 1 means material instacing TODO
		if( true || pParam->size() == 1 ) {
			const FloatScalar& pScalar = pParam->getElement(0);

			uint16_t uiFlags = pScalar.isAnimated ? WobMaterialParameter::WMPF_ANIMATED : 0;

			parameterString << "(u16)" << WobMaterialParameter::WMPT_SCALAR_FLOAT << " // SCALAR FLOAT PARAM" << "\n";
			parameterString << "(u16)" << uiFlags << " // Flags: " << (pScalar.isAnimated ? "Animated " : "") << "\n";
			// write the parameter name to the string table and insert the poitner in the parameter
			m_stringTable[ paramBaseName + ":" ] = paramBaseName;
			parameterString << "(u32)" << 0 << "\n" << paramBaseName << "\n";
			// the pointer to the data (which is packed with other param data at the end of the 
			// paramater array
			parameterString << "(u32)" << 0 << "\n" << (txtBaseName + paramBaseName + "_Data") << "\n";
			parameterDataString << (txtBaseName + paramBaseName + "_Data: \n");
			parameterDataString << pScalar.x << "// " << paramBaseName << " = " << pScalar.x << "\n";

		} else
		{
			// TODO
			assert(false);
		}
	}

	return (unsigned int) fSPContainer.size();
}

uint32_t WobbyWriter::WriteRGBParams(	const MeshMod::MaterialData::ParameterContainer& params, 
										const std::string& txtBaseName,
										std::ostringstream& parameterString,
										std::ostringstream& parameterDataString) {
	using namespace MeshMod;
	using namespace Scene;

	std::vector< const MaterialData::RGBElements* > fRGBPContainer;
	params.getAllElements( fRGBPContainer );

	for( unsigned int i=0;i < fRGBPContainer.size();++i) {
		using namespace MaterialData;
		const RGBElements* pParam = fRGBPContainer[i];
		std::string paramBaseName = pParam->subName;
		// the usual case is one element > 1 means material instacing TODO
		if( true || pParam->size() == 1 ) {
			const RGBColour& pRGB = pParam->getElement(0);

			uint16_t uiFlags = pRGB.isAnimated ? WobMaterialParameter::WMPF_ANIMATED : 0;

			parameterString << "(u16)" << WobMaterialParameter::WMPT_VEC3_FLOAT << " // (RGB) VEC3 FLOAT PARAM" << "\n";
			parameterString << "(u16)" << uiFlags << " // Flags: " << (pRGB.isAnimated ? "Animated " : "") << "\n";
			// write the parameter name to the string table and insert the poitner in the parameter
			m_stringTable[ paramBaseName + ":" ] = paramBaseName;
			parameterString << "(u32)" << 0 << "\n" << paramBaseName << "\n";
			// the pointer to the data (which is packed with other param data at the end of the 
			// paramater array
			parameterString << "(u32)" << 0 << "\n" << (txtBaseName + paramBaseName + "_Data") << "\n";
			parameterDataString << (txtBaseName + paramBaseName + "_Data: \n");
			parameterDataString <<	pRGB.r << ", " << pRGB.g << ", " << pRGB.b << "\t// " << 
									paramBaseName << " = " << pRGB.r << ", " << pRGB.g << ", " << pRGB.b << "\n";

		} else {
			// TODO
			assert(false);
		}
	}
	return (unsigned int) fRGBPContainer.size();
}

uint32_t WobbyWriter::WriteTextureParams(	const MeshMod::MaterialData::ParameterContainer& params, 
											const std::string& txtBaseName,
											std::ostringstream& parameterString,
											std::ostringstream& parameterDataString)
{
	using namespace MeshMod;
	using namespace Scene;
	using namespace MaterialData;

	std::vector< const TextureParameterElements* > texPContainer;
	params.getAllElements( texPContainer );

	for( unsigned int i=0;i < texPContainer.size();++i)
	{
		const TextureParameterElements* pParam = texPContainer[i];
		std::string paramBaseName = pParam->subName;
		// the usual case is one element > 1 means material instacing TODO
		if( true || pParam->size() == 1 ) {
			const TextureParameter& texture = pParam->getElement(0);

			uint16_t uiFlags = texture.isAnimated ? WobMaterialParameter::WMPF_ANIMATED : 0;

			switch( texture.type ) {
				case TextureParameter::TT_CUBE:
					parameterString << "(u16)" << WobMaterialParameter::WMPT_TEXTURE_CUBE_MAP << " // TEXTURE CUBE MAP PARAM" << "\n";
					break;
				case TextureParameter::TT_1D:
					parameterString << "(u16)" << WobMaterialParameter::WMPT_TEXTURE_1D << " // TEXTURE 1D PARAM" << "\n";
					break;
				case TextureParameter::TT_2D:
					parameterString << "(u16)" << WobMaterialParameter::WMPT_TEXTURE_2D << " // TEXTURE 2D PARAM" << "\n";
					break;
				case TextureParameter::TT_3D:
					parameterString << "(u16)" << WobMaterialParameter::WMPT_TEXTURE_3D << " // TEXTURE 3D PARAM" << "\n";
					break;
			}
			parameterString << "(u16)" << uiFlags << " // Flags: " << (texture.isAnimated ? "Animated " : "") << "\n";
			// write the parameter name to the string table and insert the poitner in the parameter
			m_stringTable[ paramBaseName + ":" ] = paramBaseName;
			parameterString << "(u32)" << 0 << "\n" << paramBaseName << "\n";
			// the pointer to the data (which is packed with other param data at the end of the 
			// paramater array
			parameterString << "(u32)" << 0 << "\n" << (txtBaseName + paramBaseName + "_Data") << "\n";
			parameterDataString << (txtBaseName + paramBaseName + "_Data: \n");
			parameterDataString << "\"" << texture.textureName.c_str() << "\\0\""  << " // " << paramBaseName << " = " << texture.textureName.c_str() << "\n";
		} else
		{
			// TODO
			assert(false);
		}
	}

	return (unsigned int) texPContainer.size();
}


void WobbyWriter::ConvertLightParamsToMaterialParameters( 	MeshMod::MaterialElementsContainer& matCon, 
                                                         	const MeshMod::LightParamsMaterialElements* lightElements ) {
	using namespace MeshMod;
	using namespace MeshMod::MaterialData;
	ShaderMaterialElements* shaderEle = matCon.getOrAddElements<ShaderMaterialElements>();
	MaterialParameterElements* paramEle = matCon.getOrAddElements<MaterialParameterElements>();

	for( MaterialIndex matIndex =0;matIndex < matCon.size();++matIndex ) {
		ParameterContainer& paramCon = (*paramEle)[ matIndex ].parameters;
		const MaterialData::LightParams& params = (*lightElements)[ matIndex ];

		// ignore lightparams if this material already has a valid shader element
		if( (*shaderEle)[ matIndex ].shaderName.empty() ) {
			// no shader convert it from  light params
			(*shaderEle)[ matIndex ] = std::string("blinn");
			paramCon.pushBack<RGBElements>( "EmmisiveColour", RGBColour(params.luminosity[0], params.luminosity[1], params.luminosity[2] ) );
			paramCon.pushBack<RGBElements>( "DiffuseColour", RGBColour(params.diffuse[0], params.diffuse[1], params.diffuse[2] ) );
			paramCon.pushBack<RGBElements>( "SpecularColour", RGBColour(params.specular[0], params.specular[1], params.specular[2] ) );
			paramCon.pushBack<FloatScalarElements>( "Shininess", FloatScalar( params.specular_exponent ) );
		}
	}
}


void WobbyWriter::WriteMaterial( const unsigned int matNum, const unsigned int uiFlags, std::ostream& outStream  ) {
	using namespace MeshMod;
	using namespace Scene;

	// its legal for the material name to be an illegal binify label
	// so we just refer to it as Mat0 for labels.
	std::ostringstream matNameMaker;
	matNameMaker << "Mat" << matNum;
	std::string matName = matNameMaker.str();

	VertexElementsContainer& vertCon = m_goMesh->getVertexContainer();
	FaceElementsContainer& faceCon = m_goMesh->getFaceContainer();
	MaterialElementsContainer& matCon = m_goMesh->getMaterialContainer();

	const NameMaterialElements* nameEle = matCon.getElements<NameMaterialElements>();
	const ShaderMaterialElements* shaderEle = matCon.getElements<ShaderMaterialElements>();
	const MaterialParameterElements* parameterElements = matCon.getElements<MaterialParameterElements>();
	const VertexBindingsElements* vbindEle = matCon.getOrAddElements<VertexBindingsElements>();

	const PositionVertexElements*	posEle = vertCon.getElements<PositionVertexElements>();
	const NormalVertexElements*	normEle = vertCon.getElements<NormalVertexElements>();
	const UVVertexElements*	uvEle = vertCon.getElements<UVVertexElements>();
	const BoneWeightsVertexElements* boneEle = vertCon.getElements<BoneWeightsVertexElements>();

	// mixed shader and light params input (light params will be converted into shaders where there is no shader set)
	const LightParamsMaterialElements* lightElements = matCon.getElements<LightParamsMaterialElements>();
	if( lightElements != NULL ) {
		ConvertLightParamsToMaterialParameters( matCon, lightElements );
		shaderEle = matCon.getElements<ShaderMaterialElements>();
		parameterElements = matCon.getElements<MaterialParameterElements>();
	} else if( shaderEle == NULL ) {
		assert( false );// unknown material type
	}

//	assert( (boneEle != 0) && (uiFlags & WF_SKINNED) );

	//-- material name
	m_stringTable[ matName  + ":" ] = (*nameEle)[matNum].matName;
	outStream << ".align 8\n";
	outStream << "0, " << matName << "// " << (*nameEle)[matNum].matName << "\n";				// string table address

	//-- shader 
	if( shaderEle ) {
		m_stringTable[ (*shaderEle)[matNum].shaderName + ":" ] = (*shaderEle)[matNum].shaderName;
		outStream << "0, " << (*shaderEle)[matNum].shaderName << "\n";				// string table address
	} else {
		assert( false );
	}
	// just a nice label to make the txtwob more readable and this becomes
	// the start of the param array
	std::ostringstream parameterLabel;
	parameterLabel << "Mat" << matNum << "_Params";

	// the parameter string array is a continous array of WobMaterialParams
	std::ostringstream parameterString;
	// cos data for each param itself is variable length its put at the end and pointed to
	// by the material param itself (yer its one extra pointer and deref per param but
	// it will be cached if 4 bytes per param shouldn't kill us...)
	std::ostringstream parameterDataString;

	// write each type of parameter out in turn...
	// make the default float
	parameterDataString << "//--------- PARAMETER DATA -----------\n";
	parameterDataString << ".type float\n";
	parameterString << ".type u32\n";

	uint32_t numParams = 0;
	if( parameterElements ) {
		// material parameters
		const MaterialData::ParameterContainer& params = (*parameterElements)[matNum].parameters;

		std::string txtParamBaseName = parameterLabel.str();
		numParams += WriteScalarParams( params, txtParamBaseName , parameterString, parameterDataString );
		numParams += WriteRGBParams( params, txtParamBaseName , parameterString, parameterDataString );
		numParams += WriteTextureParams( params, txtParamBaseName , parameterString, parameterDataString );
	}
	parameterDataString << ".type u32\n";

	//- vertex descriptor
	// vertex element array
	std::ostringstream vertexElementDataString;
	std::ostringstream vertexElementLabel;
	vertexElementLabel << "Mat" << matNum << "_VertexElements";

	unsigned int numUsedVertexTypes = 0;
	vertexElementDataString << "//vertex types\n .type u16\n";						// 16 bit integer for the vertex type table

	// fallback case
	if( (*vbindEle)[ matNum].numVertexBindings == 0 ) {
		vertexElementDataString << WobVertexElement::WVTU_POSITION << "," << WobVertexElement::WVTT_FLOAT3 << "\n";
		vertexElementDataString << WobVertexElement::WVTU_NORMAL << "," << WobVertexElement::WVTT_FLOAT3 << "\n";
		numUsedVertexTypes+=2;
		if( uvEle != NULL ) {
			vertexElementDataString << WobVertexElement::WVTU_TEXCOORD0 << "," << WobVertexElement::WVTT_FLOAT2 << "\n";
			numUsedVertexTypes++;
		}
	} else {
		assert( false ); // need to check
	}

	if( uiFlags & WF_SKINNED && boneEle != 0 ) {
		vertexElementDataString << WobVertexElement::WVTU_BONEINDICES << "," << WobVertexElement::WVTT_BYTEARGB << "\n";
		vertexElementDataString << WobVertexElement::WVTU_BONEWEIGHTS << "," << WobVertexElement::WVTT_FLOAT4 << "\n";
		numUsedVertexTypes+=2;
	}
	vertexElementDataString << ".type u32\n";
	// tap it next to the parameter data table
	m_parameterDataTable[ parameterLabel.str() + ":" ] = parameterString.str();
	m_parameterDataTable[ vertexElementLabel.str() + ":" ] = vertexElementDataString.str();
	m_parameterDataTable[ parameterLabel.str() + "_data:" ] = parameterDataString.str();

	uint16_t flags = 0;
	if( m_vertListArray[matNum].size() > (64*1024) ) {
		flags |= WobMaterial::WM_32BIT_INDICES;
	}
	outStream << "(u8) " << numUsedVertexTypes << "\t//num vertex elements \n";
	outStream << "(u8) " << numParams << "\t//num material parameters \n"; // how many parameters
	outStream << "(u16) " << flags << "\t//material flags\n";
	outStream << (uint32_t) m_vertListArray[matNum].size() << "\t//numVertices\n";
	outStream << (uint32_t) m_faceListArray[matNum].size()*3 << "\t//numIndices\n";

	float minAABB[3], maxAABB[3];
	minAABB[0] = minAABB[1] = minAABB[2] = FLT_MAX;
	maxAABB[0] = maxAABB[1] = maxAABB[2] = -FLT_MAX;
	CalcMaterialAABB( matNum, posEle, minAABB, maxAABB );

	outStream << ".type float\n";
	outStream << minAABB[0] << ", " << minAABB[1] << ", " << minAABB[2] << "\t//minAABB\n";
	outStream << maxAABB[0] << ", " << maxAABB[1] << ", " << maxAABB[2] << "\t//maxAABB\n";
	outStream << ".type u32\n";
	outStream << ".align 8\n";

	outStream << "0, " << vertexElementLabel.str() << "\n";
	// store parameter label
	outStream << "0, " << parameterLabel.str() << "\n";

	WriteVerticesAndIndices( matNum, posEle,normEle,uvEle, boneEle, outStream );

}

void WobbyWriter::CalcMaterialAABB( const unsigned int matNum, const MeshMod::PositionVertexElements* posEle, float outMin[3], float outMax[3] ) {

	std::set<uint32_t>::const_iterator setIt = m_vertListArray[matNum].begin();
	while( setIt != m_vertListArray[matNum].end() ) {
		outMin[0] = Math::Min( (*posEle)[(*setIt)].x, outMin[0] );
		outMin[1] = Math::Min( (*posEle)[(*setIt)].y, outMin[1] );
		outMin[2] = Math::Min( (*posEle)[(*setIt)].z, outMin[2] );
		outMax[0] = Math::Max( (*posEle)[(*setIt)].x, outMax[0] );
		outMax[1] = Math::Max( (*posEle)[(*setIt)].y, outMax[1] );
		outMax[2] = Math::Max( (*posEle)[(*setIt)].z, outMax[2] );
		++setIt;
	}
}

void WobbyWriter::CalcMeshAAB( float outMin[3], float outMax[3] ) {
	using namespace MeshMod;
	VertexElementsContainer& vertCon = m_goMesh->getVertexContainer();
	MaterialElementsContainer& matCon = m_goMesh->getMaterialContainer();

	const NameMaterialElements* nameEle = matCon.getElements<NameMaterialElements>();
	const PositionVertexElements*	posEle = vertCon.getElements<PositionVertexElements>();

	outMin[0] = outMin[1] = outMin[2] = FLT_MAX;
	outMax[0] = outMax[1] = outMax[2] = -FLT_MAX;

	// for each material
	NameMaterialElements::const_iterator nameIt = nameEle->elements.begin();
	while( nameIt != nameEle->elements.end() )
	{
		const MaterialIndex matNum = (const MaterialIndex) 
			std::distance<NameMaterialElements::const_iterator>( nameEle->elements.begin(), nameIt );

		// skip materials that aren't used
		if( m_faceListArray[matNum].size() > 0 ) {
			CalcMaterialAABB( matNum, posEle, outMin, outMax );
		}
		++nameIt;
	}

}

void WobbyWriter::WriteVerticesAndIndices(	unsigned int matNum,
								const MeshMod::PositionVertexElements*						posEle,
								const MeshMod::NormalVertexElements*						normEle,
								const MeshMod::UVVertexElements*							uvEle,
								const MeshMod::BoneWeightsVertexElements*					boneEle,
								std::ostream& outStream )
{
	// build vertex data block
	unsigned int vertIndex = 0;
	std::map< uint32_t, uint32_t > vertexRemap;

	std::ostringstream vertexLabel;
	std::ostringstream vertexString;

	vertexLabel << "VertexData" << matNum;
	vertexString << ".type float\n";

	std::set<uint32_t>::const_iterator setIt = m_vertListArray[matNum].begin();
	while( setIt != m_vertListArray[matNum].end() ) {
		if( posEle ) {
			vertexString << (*posEle)[(*setIt)].x << "," << (*posEle)[(*setIt)].y << "," << (*posEle)[(*setIt)].z;
			vertexString << " // pos\n";
		}
		if( normEle ) {
			float x = (*normEle)[(*setIt)].x;
			float y = (*normEle)[(*setIt)].y;
			float z = (*normEle)[(*setIt)].z;
			x = isnan(x) ? 0.0f : x;
			y = isnan(y) ? 0.0f : y;
			z = isnan(z) ? 0.0f : z;

			vertexString << x  << "," << y << "," << z;
			vertexString << " // norm\n";
		}
		if( uvEle ) {
			float u = (*uvEle)[(*setIt)].u;
			float v = (*uvEle)[(*setIt)].v;
			u = isnan(u) ? 0.0f : u;
			v = isnan(v) ? 0.0f : v;
			vertexString << u << "," << v;
			vertexString << " // uv0\n"; 
		}
		if( boneEle ) {
			// copy to bone weights for this vertex, so we can sort without any damage
			std::vector<MeshMod::VertexData::BoneWeights::Weight> boneData( (*boneEle)[(*setIt)].boneData );
			std::sort( boneData.begin(), boneData.end() );
			std::reverse( boneData.begin(), boneData.end() ); // get biggest weight first

			static const int MAX_BONES_PER_VERTEX = 4;
			unsigned int indices[ MAX_BONES_PER_VERTEX ];
			float		weights[  MAX_BONES_PER_VERTEX ];
			int numWeights = 0;

			std::vector<MeshMod::VertexData::BoneWeights::Weight>::iterator bwIt = boneData.begin();
			while(	(bwIt != boneData.end()) && 
					(numWeights < MAX_BONES_PER_VERTEX) ) {
				indices[numWeights] = bwIt->index;
				weights[numWeights] = bwIt->weight;
				++numWeights;
				++bwIt;
			}
			// D3DCOLOR packing is different from UBYTE4, we are using D3DCOLOR so we can support GF3
			vertexString	<<   "(u8)" << indices[1]
							<< " , (u8)" << indices[2]
							<< " , (u8)" << indices[3]
							<< " , (u8)" << indices[0]
							<< " // D3DCOLOUR bone indices\n";
			vertexString	<<			weights[0]
							<< " , " <<	weights[1]
							<< " , " <<	weights[2]
							<< " , " <<	weights[3]
							<< " // bone weights\n";
		}

		vertexRemap[ (*setIt) ] = vertIndex;

		++setIt;
		++vertIndex;
	}
	m_vertexDataTable[ vertexLabel.str() + ":" ] = vertexString.str();

	// build index data block
	std::vector<uint32_t>::const_iterator faceIt = m_faceListArray[matNum].begin();
	MeshMod::VertexIndexContainer vertexList;

	std::ostringstream indexLabel;
	std::ostringstream indexString;
	indexLabel << "Index" << matNum;
	if( m_vertListArray[matNum].size() < (64*1024) ) {
		indexString << ".type u16\n";
		while( faceIt != m_faceListArray[matNum].end() )
		{
			vertexList.clear();
			m_goMesh->getFaceVertices( (*faceIt), vertexList );
			// TODO Points vertexList.size() == 1
			// TODO Lines vertexList.size() == 2
			if( vertexList.size() == 3 ) {
				indexString << (uint16_t) vertexRemap[ vertexList[0] ] << ","
							<< (uint16_t) vertexRemap[ vertexList[1] ] << ","
							<< (uint16_t) vertexRemap[ vertexList[2] ] << "\n";
			}
			++faceIt;
		}
	} else {
		indexString << ".type u32\n";
		while( faceIt != m_faceListArray[matNum].end() )
		{
			vertexList.clear();
			m_goMesh->getFaceVertices( (*faceIt), vertexList );
			// TODO Points vertexList.size() == 1
			// TODO Lines vertexList.size() == 2
			if( vertexList.size() == 3 ) {
				indexString << (uint32_t) vertexRemap[ vertexList[0] ] << ","
							<< (uint32_t) vertexRemap[ vertexList[1] ] << ","
							<< (uint32_t) vertexRemap[ vertexList[2] ] << "\n";
			}
			++faceIt;
		}
	}
	indexString << ".type u32\n";
	m_indexDataTable[ indexLabel.str() + ":" ] = indexString.str();

	// store index label
	outStream << "0, " << indexLabel.str() << "-DiscardBlockStart\n";
	// store vertex label
	outStream << "0, " << vertexLabel.str() << "-DiscardBlockStart\n";
	outStream << "0, 0 \t\t //place for backEndData ptr after load\n";
}

bool WobbyWriter::Save( MeshMod::MeshPtr goMesh, const Core::FilePath outFilename ) {
	using namespace MeshMod;
	using namespace Scene;
	assert( goMesh != 0 );
	std::string baseName = outFilename.BaseName().value();
	std::replace( baseName.begin(), baseName.end(), ' ', '_' );
	std::ostringstream outStream;

	// out a text binify format wob file for
	m_goMesh = goMesh;
	MeshOps::BasicMeshOps meshOp( m_goMesh );

	VertexElementsContainer& vertCon = m_goMesh->getVertexContainer();
	FaceElementsContainer& faceCon = m_goMesh->getFaceContainer();
	MaterialElementsContainer& matCon = m_goMesh->getMaterialContainer();
	if( matCon.getElementCount() == 0 ) {
		// give a default material
		LightParamsMaterialElements* lightEle = 
						matCon.addElements<LightParamsMaterialElements>();
		NameMaterialElements* nameEle = 
						matCon.addElements<NameMaterialElements>();
		// shiny mid grey default material
		nameEle->push_back( MaterialData::Name("Default") );
		MaterialData::LightParams def;
		def.diffuse[0] = 0.5f;
		def.diffuse[1] = 0.5f;
		def.diffuse[2] = 0.5f;
		def.luminosity[0] = 0;
		def.luminosity[1] = 0;
		def.luminosity[2] = 0;
		def.specular[0] = 1.0f;
		def.specular[1] = 1.0f;
		def.specular[2] = 1.0f;
		def.specular_exponent = 40.f;
		lightEle->push_back(def);
	}
	
	// get the position stream
	PositionVertexElements*	posEle = vertCon.getElements<PositionVertexElements>();
	assert( posEle != 0 );
	NormalVertexElements*	normEle = vertCon.getElements<NormalVertexElements>();
	if( normEle == 0 ) {
		meshOp.computeVertexNormals();
		// now get the normal elements
		normEle = vertCon.getElements<NormalVertexElements>();
		assert( normEle != 0 );
	}
	MaterialFaceElements* surfEle = faceCon.getElements<MaterialFaceElements>();
	if( surfEle == 0 ) {
		// if we have no surface material elements we have no mesh to work with
		return false;
	}

	meshOp.triangulate();

	// we need to produce a list of faces and unique vertices per material 
	m_vertListArray.reset( new std::set<uint32_t>[ matCon.size() ] );
	m_faceListArray.reset( new std::vector<uint32_t>[ matCon.size() ] );

	// compute the flags and mesh options this mesh uses
//	const bool bSkinned = (vertCon.getElements( VertexData::BoneWeights::getName() ) != 0);
//	const bool bMorphed = (vertCon.getElements( VertexData::PositionDelta::getName() ) != 0);
	const bool bSkinned = false;
	const bool bMorphed = false;

	NameMaterialElements* nameEle = matCon.getElements<NameMaterialElements>();
	NameMaterialElements::const_iterator nameIt = nameEle->elements.begin();

	// make the material oriented vertex and face lists
	unsigned int numMaterials = 0;
	while( nameIt != nameEle->elements.end() ) {
		const MaterialIndex matNum = nameEle->distance<MaterialIndex>( nameIt );

		VertexIndexContainer facevList;
		facevList.reserve( 10 );

		// for each material now produce the vertex and face list
		MaterialFaceElements::const_iterator surfIt = surfEle->cbegin();
		while( surfIt != surfEle->cend() ) {
			if( surfIt->surfaceIndex == matNum ) {
				// if first time we seen this material
				if( m_faceListArray[matNum].size() == 0 ) {
					// allocate some ram to minimise reallocs
					m_faceListArray[matNum].reserve( 1024 );
				}
				const FaceIndex faceNum = surfEle->distance<FaceIndex>( surfIt );
				facevList.clear();
				m_goMesh->getFaceVertices( faceNum, facevList );
				// TODO points and lines
				if( facevList.size() > 2 ) {
					m_vertListArray[matNum].insert( facevList.cbegin(), facevList.cend() );
					m_faceListArray[matNum].push_back( faceNum );
				}

			}
			++surfIt;
		}
		// lets now count the number of active material we have
		if( m_faceListArray[matNum].size() > 0 )
			++numMaterials;

		++nameIt;
	}
	// do we have any active materials?
	if(numMaterials == 0 )
		return false;

	unsigned int uiFlags = 0;
	uiFlags |= bSkinned ? WF_SKINNED : 0;
	uiFlags |= bMorphed ? WF_VERTEXMORPHS : 0;

	// make a useful comment
	std::ostringstream flagComment;
	flagComment << " // flags = ";
	flagComment << (bSkinned ? " WF_SKINNED" : "");
	flagComment << (bMorphed ? " WF_VERTEXMORPHS" : "");
	flagComment << "\n";

	// produce a wob header as we now ready to output data
	outStream << "//wob file\n";
	outStream << ".type u32\n";

	outStream << "//Start Wob Header" << "\n";
	outStream << WobType << "\t//WOB1\n";						// magic
	outStream << "(u16)" << WobVersion << "\t//Version\n";				// WobVersion
	outStream << "(u16)" << numMaterials << "\t//numMaterials\n";		// number of materials
	outStream << "(u32)" << uiFlags << flagComment.str();				// flags
	//-- wob name
	m_stringTable[ baseName  + ":" ] = baseName;
	outStream << ".align 8\n";
	outStream << "0, " << baseName << "// " << baseName << "\n";

	float minAABB[3], maxAABB[3];
	CalcMeshAAB( minAABB, maxAABB );
	outStream << ".type float\n";
	outStream << minAABB[0] << ", " << minAABB[1] << ", " << minAABB[2] << "\t //minAABB\n";
	outStream << maxAABB[0] << ", " << maxAABB[1] << ", " << maxAABB[2] << "\t //maxAABB\n";
	outStream << ".type u32\n";

	outStream << ".align 8\n";
	outStream << "0, " << "MaterialListStart" << "\n";							// label where our materials start
	outStream << "MainBlockEnd-MainBlockStart" << "\n";					// calculated sizeof main block
	outStream << "DiscardBlockEnd-DiscardBlockStart" << "\n";			// calculated sizeof discard block
	outStream << "//End Wob Header" << "\n";

	outStream <<"MainBlockStart:\n";
	// output the material list state label
	outStream << "MaterialListStart:" << "\n";

	//
	// for each material
	nameIt = nameEle->elements.begin();
	while( nameIt != nameEle->elements.end() )
	{
		const MaterialIndex matNum = (const MaterialIndex) 
			std::distance<NameMaterialElements::const_iterator>( nameEle->elements.begin(), nameIt );

		// skip materials that aren't used
		if( m_faceListArray[matNum].size() > 0 ) {
			WriteMaterial( matNum, uiFlags, outStream );
		}
		++nameIt;
	}

	outStream << "// string table start\n";
	// now output the data tables
	std::map<std::string,std::string>::const_iterator stIt = m_stringTable.begin();
	while( stIt != m_stringTable.end() )
	{
		outStream << stIt->first << "\n" << "\"" << stIt->second << "\\0\"" << "\n";
		++stIt;;
	}
	outStream << "// parameter table start\n";
	std::map<std::string,std::string>::const_iterator ptIt = m_parameterDataTable.begin();
	while( ptIt != m_parameterDataTable.end() )
	{
		outStream << ptIt->first << "\n" << ptIt->second << "\n";
		++ptIt;
	}
	outStream <<"MainBlockEnd:\n";

	//-----------------------------------------------------------------
	//-----------------------------------------------------------------
	// all data past this point in the file will be discarded after load
	// so obviously nothing required while running should be store post
	// here
	//-----------------------------------------------------------------
	//-----------------------------------------------------------------
	outStream <<"DiscardBlockStart:\n";
	outStream << "// vertex table start\n";
	std::map<std::string,std::string>::const_iterator vtIt = m_vertexDataTable.begin();
	while( vtIt != m_vertexDataTable.end() )
	{
		outStream << vtIt->first << "\n" << vtIt->second << "\n";
		++vtIt;
	}
	outStream << "// index table start\n";
	std::map<std::string,std::string>::const_iterator itIt = m_indexDataTable.begin();
	while( itIt != m_indexDataTable.end() )
	{
		outStream << itIt->first << "\n" << itIt->second << "\n";
		++itIt;
	}
	outStream <<"DiscardBlockEnd:\n";


	// add a Manifest folder to the path
	auto filedir = outFilename.DirName();
	filedir = filedir.Append( "Meshes" );
	std::string fileName( outFilename.BaseName().value() );
	std::replace( fileName.begin(), fileName.end(), ' ', '_' );

	filedir = filedir.Append( baseName );

//#if defined(_DEBUG)
	std::ofstream foutStream;
	auto wobtextpath = filedir.ReplaceExtension( ".wobtxt" );
	foutStream.open( wobtextpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();
//#endif

	auto wobpath = filedir.ReplaceExtension( ".wob" );
	foutStream.open( wobpath.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();

	return true;
}

bool WobbyWriter::Save( MeshMod::SceneNodePtr node, Core::ResourceManifestEntryVector& manifest, const Core::FilePath pOutFilename ) {
	using namespace MeshMod;
	using namespace Scene;

	// write Node Header

	// write each mesh out...
	for( unsigned int j=0;j < node->getObjectCount();++j ) {
		MeshMod::SceneObjectPtr pObj = node->getObject(j);

		if( pObj->getType() == "mesh" || pObj->getType() == "Mesh" ) {
			MeshMod::MeshPtr pMesh = std::dynamic_pointer_cast<MeshMod::Mesh>(pObj);

			WobbyWriter oWriter;
			auto baseName = Core::FilePath( pOutFilename.BaseName().RemoveExtension().value() + "_" + pMesh->getName() );
			auto fileName = baseName.RemoveExtension();
			oWriter.Save( pMesh, fileName );

			manifest.push_back( Core::ResourceManifestEntry( WobType, fileName.value() ) );
		}
	}
	// write tree
	for( unsigned int i=0;i  < node->getChildCount(); ++i ) {
		MeshMod::SceneNodePtr cnode = node->getChild(i);
		Save( cnode, manifest, pOutFilename );
	}
	return true;
}

// simple C like API 
bool SaveMeshes( MeshMod::SceneNodePtr root, Core::ResourceManifestEntryVector& manifest, const Core::FilePath pOutFilename ) {
	WobbyWriter oWriter;
	return oWriter.Save( root, manifest, pOutFilename );
}


} // end namespace MeshExport