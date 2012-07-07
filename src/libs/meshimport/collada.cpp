// collada.cpp
//

#include "meshimport.h"

#if 0

#include <iostream>
#include <fstream>

#include <meshmod/materialparameter.h>
#include <meshmod/materialvertexbindings.h>
#include <meshmod/mesh.h>
#include <meshmod/scene.h>
#include <meshmod/sceneobject.h>
#include <meshmod/uvvertex.h>
#include <meshmod/rgbmaterialparameter.h>
#include <meshmod/scalarmaterialparameter.h>
#include <meshmod/texturematerialparameter.h>
#include <meshmod/materialFace.h>

#include <dae.h>
#include <dae/daeUtils.h>
#include <dom/domCOLLADA.h>
#include <dom/domAsset.h>
#include <dom/domCamera.h>
#include <dom/domMaterial.h>
#include <dom/domProfile_common.h>
#include <dom/domMesh.h>

#include "collada.h"

static std::map<std::string, uint32_t> s_veaLookup;

// Our mesh structure, which we create by converting a domGeometry object
class Mesh {
public:
	MeshMod::MeshPtr		mesh;
	class Material* mtl;
	// Vertex info, etc

	Mesh(domGeometry& geom);

private:
	void AddPolygonsFromVertexSource(  const domMesh* pInMesh );
	bool ExtractSourceInput( const domInputLocalOffset* inputOffset, uintptr_t vertexExtraAttribsHelper[3] );
	void ExtractFaceAndVertexExtraData(	const unsigned int numIndices, 
											const uint32_t numVertexExtraAttribs, 
											const uintptr_t vertexExtraAttribsHelper[16][3],
											const unsigned int firstIndex,
											const domListOfUInts& polyIndexData,
											const MeshMod::MaterialIndex matIndex );

	static MeshMod::VertexIndexContainer vertexIndices;

	MeshMod::NormalVertexElements* normEle;
	MeshMod::UVVertexElements* uvEle;
	MeshMod::MaterialFaceElements* faceMatEle;
};
MeshMod::VertexIndexContainer Mesh::vertexIndices;

// Our material structure, which we create by converting a domMaterial object
class Material {
public:
	Material(const domMaterial& _mtl) :
		m_mtl( _mtl ) {
		// Grab the <effect> from the <material> and initalize the parameters
	}

	void addMesh( const domInstance_material* instanceMtl, Mesh* meshWrap );
	void fillInLambertMaterial( MeshMod::MeshPtr mesh, MeshMod::MaterialIndex matNum, const domProfile_COMMON::domTechnique::domLambert* lamb );
	void fillInBlinnMaterial( MeshMod::MeshPtr mesh, MeshMod::MaterialIndex matNum, const domProfile_COMMON::domTechnique::domBlinn* blinn );
	void fillInDefaultMaterial( MeshMod::MeshPtr mesh, MeshMod::MaterialIndex matNum );
	void addSampler( const std::string& samplerName, const std::string& paramName, MeshMod::MaterialData::ParameterContainer& paramCon );

	const domMaterial&	m_mtl;
	std::list< Mesh* >	m_meshList;
	struct SamplerTextureLink {
		SamplerTextureLink() {}
		SamplerTextureLink( const domCommon_newparam_type* _sampler, const domFx_surface_init_common* _texture ) :
			sampler(_sampler), init_common(_texture) {};
		const domCommon_newparam_type* sampler;
		const domFx_surface_init_common* init_common;
	};

	std::map< std::string, SamplerTextureLink  > m_samplers;
};

void Material::addMesh( const domInstance_material* instanceMtl, Mesh* meshWrap ) {
	MeshMod::MeshPtr mesh = meshWrap->mesh;
	// see if we already have this mesh attached to this material
	if( std::find( m_meshList.begin(), m_meshList.end(), meshWrap ) == m_meshList.end() ) {
		m_meshList.push_back( meshWrap );

		MeshMod::MaterialIndex matNum = mesh->findMaterial( instanceMtl->getSymbol() );
		assert( matNum != MeshMod::GOM_INVALID_INDEX );

		domInstance_effect* insEffect = m_mtl.getInstance_effect();
		domEffect* effect = daeSafeCast<domEffect>( insEffect->getUrl().getElement() );

		domProfile_COMMON* profile = NULL;
		for( unsigned int i=0;i < effect->getFx_profile_abstract_array().getCount();++i) {
			domFx_profile_abstract* abstractProfile = effect->getFx_profile_abstract_array()[i];
			if( abstractProfile->typeID() == domProfile_COMMON::ID() ) {
				profile = (domProfile_COMMON*)abstractProfile;
				break;
			}
		}

		if( profile != NULL ) {
			using namespace GO;
			using namespace MeshMod::MaterialData;
			// lets push in the shared parameters
			MaterialElementsContainer& matCon = mesh->getMaterialContainer();
			MaterialParameterElements* paramEle = matCon.getOrAddElements<MeshMod::MaterialParameterElements>();
			ParameterContainer& paramCon = (*paramEle)[ matNum ].m_parameters;
			for( size_t i=0;i < profile->getNewparam_array().getCount();++i ) {
				domCommon_newparam_type* newparam = profile->getNewparam_array()[i];
				// work out what type of parameter is it...
				if(newparam->getSemantic() != NULL ) {
					//domSemantic* semantic = newparam->getSemantic();
					// don't know what to do yet, so skip
				} else if( newparam->getFloat() != NULL ) {
					paramCon.pushBack<FloatScalarElements>( newparam->getID(), FloatScalar( newparam->getFloat()->getValue() ) );
				} else if( newparam->getFloat2() != NULL ) {
				} else if( newparam->getFloat3() != NULL ) {
				} else if( newparam->getFloat4() != NULL ) {
				} else if( newparam->getSampler2D() != NULL ) {

					const domFx_sampler2D_common* sampler = newparam->getSampler2D();
					const std::string sourceName = sampler->getSource()->getValue();
					const domFx_surface_common* surface = NULL;
					// scan through new params array search for a surface type with sid == sourceName
					for( size_t j=0;j < profile->getNewparam_array().getCount();++j ) {
						if( profile->getNewparam_array()[j]->getSurface() != NULL ) {
							if( sourceName == profile->getNewparam_array()[j]->getSid() ) {
								surface = profile->getNewparam_array()[j]->getSurface();
								break;
							} 
						}
					}

					if( surface == NULL ) {
						return;
					}

					const domFx_surface_init_common* init_common = surface->getFx_surface_init_common();

					// if we have an array just grab the first, TODO image sequences
					if ( init_common != NULL && init_common->getInit_from_array().getCount() > 0 ) {
						m_samplers[ newparam->getSid() ] = SamplerTextureLink( newparam, init_common );
					}

				} else if( newparam->getSurface() != NULL ) {
					// ignore surfaces directly and instead access then via a sampler
				} else {
					// unknown skip...
				}

			}

			const domProfile_COMMON::domTechnique* technique = profile->getTechnique();
			if( technique->getBlinn() != NULL ) {
				fillInBlinnMaterial( mesh, matNum, technique->getBlinn() );
			} else if( technique->getPhong() != NULL ) {
				fillInDefaultMaterial( mesh, matNum );
			} else if( technique->getLambert() != NULL ) {
				fillInLambertMaterial( mesh, matNum, technique->getLambert() );
			} else if( technique->getConstant() != NULL ) {
				fillInDefaultMaterial( mesh, matNum );
			} else {
				fillInDefaultMaterial( mesh, matNum );
			}
		} else {
			fillInDefaultMaterial( mesh, matNum );
		}
		meshWrap->mtl = this;
	}
}
void Material::addSampler( const std::string& samplerName, const std::string& paramName, MeshMod::MaterialData::ParameterContainer& paramCon ) {
	using namespace GO;
	using namespace MeshMod::MaterialData;
	if( m_samplers.find( samplerName ) == m_samplers.end() ) {
		return;
	}

	const SamplerTextureLink& stl = m_samplers[ samplerName ];

	const domImage* image = daeSafeCast<domImage>( stl.init_common->getInit_from_array()[0]->getValue().getElement() );
	xsAnyURI& url = image->getInit_from()->getValue();
	paramCon.pushBack<TextureParameterElements>( paramName + "Texture", TextureParameter( TextureParameter::TT_2D, url.pathFileBase() ) );

}
void Material::fillInBlinnMaterial( MeshMod::MeshPtr mesh, MeshMod::MaterialIndex matNum, const domProfile_COMMON::domTechnique::domBlinn* blinn ) {
	using namespace GO;
	using namespace MeshMod::MaterialData;

	MaterialElementsContainer& matCon = mesh->getMaterialContainer();
	ShaderMaterialElements* shaderEle = matCon.getOrAddElements<ShaderMaterialElements>();
	MaterialParameterElements* paramEle = matCon.getOrAddElements<MaterialParameterElements>();

	ParameterContainer& paramCon = (*paramEle)[ matNum ].m_parameters;


	std::string shaderName = "blinn";

	// work out which textures this blinn surface uses
	if( blinn->getAmbient()->getTexture() != NULL ) {
		shaderName += "_at";
		addSampler( blinn->getAmbient()->getTexture()->getTexture(), "Ambient", paramCon );
	} else {
		paramCon.pushBack<RGBElements>( "AmbientColour", RGBColour( blinn->getAmbient()->getColor()->getValue()[0],
																	blinn->getAmbient()->getColor()->getValue()[1],
																	blinn->getAmbient()->getColor()->getValue()[2] ) );
	}
	if( blinn->getDiffuse()->getTexture() != NULL ) {
		shaderName += "_dt";
		addSampler( blinn->getDiffuse()->getTexture()->getTexture(), "Diffuse", paramCon );
	} else {
		paramCon.pushBack<RGBElements>( "DiffuseColour", RGBColour( blinn->getDiffuse()->getColor()->getValue()[0],
																	blinn->getDiffuse()->getColor()->getValue()[1],
																	blinn->getDiffuse()->getColor()->getValue()[2] ) );
	}

	(*shaderEle)[ matNum ] = shaderName;
//	paramCon.pushBack<RGBElements>( "EmmisiveColour", RGBColour(.luminosity[0], params.luminosity[1], params.luminosity[2] ) );
//	paramCon.pushBack<RGBElements>( "DiffuseColour", RGBColour(params.diffuse[0], params.diffuse[1], params.diffuse[2] ) );
//	paramCon.pushBack<RGBElements>( "SpecularColour", RGBColour(params.specular[0], params.specular[1], params.specular[2] ) );
//	paramCon.pushBack<FloatScalarElements>( "Shininess", FloatScalar( params.specular_exponent ) );
}

void Material::fillInLambertMaterial( MeshMod::MeshPtr mesh, MeshMod::MaterialIndex matNum, const domProfile_COMMON::domTechnique::domLambert* lamb ) {
	using namespace GO;
	using namespace MeshMod::MaterialData;

	MaterialElementsContainer& matCon = mesh->getMaterialContainer();
	ShaderMaterialElements* shaderEle = matCon.getOrAddElements<ShaderMaterialElements>();
	MaterialParameterElements* paramEle = matCon.getOrAddElements<MaterialParameterElements>();

	ParameterContainer& paramCon = (*paramEle)[ matNum ].m_parameters;


	std::string shaderName = "blinn";

	// work out which textures this blinn surface uses
	if( lamb->getAmbient()->getTexture() != NULL ) {
		shaderName += "_at";
		addSampler( lamb->getAmbient()->getTexture()->getTexture(), "Ambient", paramCon );
	} else {
		paramCon.pushBack<RGBElements>( "AmbientColour", RGBColour( lamb->getAmbient()->getColor()->getValue()[0],
																	lamb->getAmbient()->getColor()->getValue()[1],
																	lamb->getAmbient()->getColor()->getValue()[2] ) );
	}
	if( lamb->getDiffuse()->getTexture() != NULL ) {
		shaderName += "_dt";
		addSampler( lamb->getDiffuse()->getTexture()->getTexture(), "Diffuse", paramCon );
	} else {
		paramCon.pushBack<RGBElements>( "DiffuseColour", RGBColour( lamb->getDiffuse()->getColor()->getValue()[0],
																	lamb->getDiffuse()->getColor()->getValue()[1],
																	lamb->getDiffuse()->getColor()->getValue()[2] ) );
	}

	(*shaderEle)[ matNum ] = shaderName;
//	paramCon.pushBack<RGBElements>( "EmmisiveColour", RGBColour(.luminosity[0], params.luminosity[1], params.luminosity[2] ) );
//	paramCon.pushBack<RGBElements>( "DiffuseColour", RGBColour(params.diffuse[0], params.diffuse[1], params.diffuse[2] ) );
//	paramCon.pushBack<RGBElements>( "SpecularColour", RGBColour(params.specular[0], params.specular[1], params.specular[2] ) );
//	paramCon.pushBack<FloatScalarElements>( "Shininess", FloatScalar( params.specular_exponent ) );
}


void Material::fillInDefaultMaterial( MeshMod::MeshPtr mesh, MeshMod::MaterialIndex matNum ) {
	MeshMod::MaterialElementsContainer& matCon = mesh->getMaterialContainer();
	MeshMod::LightParamsMaterialElements* lightEle = matCon.getOrAddElements<MeshMod::LightParamsMaterialElements>();
	MeshMod::BackFaceMaterialElements* backfaceEle = matCon.getOrAddElements<MeshMod::BackFaceMaterialElements>(); 

	// red base colour, as we don't have a real materials
	float baseColour[3] = { 1,0,0 }; 

	// fill in the lighting parameter
	(*lightEle)[ matNum ].diffuse[0] = baseColour[0];
	(*lightEle)[ matNum ].diffuse[1] = baseColour[1];
	(*lightEle)[ matNum ].diffuse[2] = baseColour[2];
	(*lightEle)[ matNum ].luminosity[0] = 0;
	(*lightEle)[ matNum ].luminosity[1] = 0;
	(*lightEle)[ matNum ].luminosity[2] = 0;
	(*lightEle)[ matNum ].specular[0] = baseColour[0];
	(*lightEle)[ matNum ].specular[1] = baseColour[1];
	(*lightEle)[ matNum ].specular[2] = baseColour[2];

	(*lightEle)[ matNum ].specular_exponent = 40.f;

	// back face culling
	(*backfaceEle)[ matNum ].facing = MeshMod::MaterialData::BackFace::BACK;
}


// Our node structure, which we create by converting a domNode object
class Node : public MeshMod::SceneNode {
public:

	// This is defined later to work around a circular dependency on the lookup function
	Node(domNode& node);
};


// This function checks to see if a user data object has already been attached to
// the DOM object. If so, that object is casted from void* to the appropriate type
// and returned, otherwise the object is created and attached to the DOM object
// via the setUserData method.
template<typename MyType, typename DomType>
MyType& lookup(DomType& domObject) {
	if (!domObject.getUserData()) {
		domObject.setUserData(CORE_NEW MyType(domObject));
	}
	return *(MyType*)(domObject.getUserData());
}

// This function traverses all the DOM objects of a particular type and frees
// destroys the associated user data object.
template<typename MyType, typename DomType>
void freeConversionObjects(DAE& dae) {
	std::vector<daeElement*> elts = dae.getDatabase()->typeLookup(DomType::ID());
	for (size_t i = 0; i < elts.size(); i++) {
		CORE_DELETE (MyType*)elts[i]->getUserData();
		elts[i]->setUserData( NULL );
	}
}

bool Mesh::ExtractSourceInput( const domInputLocalOffset* inputOffset, uintptr_t vertexExtraAttribsHelper[3] ) {
	xsNMTOKEN semantic = inputOffset->getSemantic();

	const domURIFragmentType& vertexURI = inputOffset->getSource();
	const domSource* source = daeSafeCast<domSource>( vertexURI.getElement() );

	// only do non position attributes
	if( source && (strcmp( semantic, daeString("POSITION") ) != 0) && (strcmp( semantic, daeString("VERTEX") ) != 0) ) {
		assert( source != NULL );
		vertexExtraAttribsHelper[0] = (uintptr_t) s_veaLookup[ semantic ];
		vertexExtraAttribsHelper[1] = (uintptr_t) source->getTechnique_common()->getAccessor()->getStride();
		vertexExtraAttribsHelper[2] = (uintptr_t) source->getFloat_array()->getValue().getRaw(0);
		switch( vertexExtraAttribsHelper[0] ) {
			case 1:  // NORMAL 								
				// lets keep it simple by assuming 3D normals...
				assert( vertexExtraAttribsHelper[1] == 3 );
				if( normEle == NULL ) {
					MeshMod::VertexElementsContainer& vertCon = mesh->getVertexContainer();
					normEle = vertCon.getOrAddElements<MeshMod::NormalVertexElements>();
				}
				break;
			case 2:  // TEXCOORD
				// lets keep it simple by assuming 2D UV...
				assert( vertexExtraAttribsHelper[1] == 2 );
				// todo multiple uv coordinates
				if( uvEle == NULL ) {
					MeshMod::VertexElementsContainer& vertCon = mesh->getVertexContainer();
					uvEle = vertCon.getOrAddElements<MeshMod::UVVertexElements>();
				}
				break;
			default:
				break;
		}

		return true;
	} else {
		return false;
	}
}

void Mesh::ExtractFaceAndVertexExtraData(	const unsigned int numIndices, 
											const uint32_t numVertexExtraAttribs, 
											const uintptr_t vertexExtraAttribsHelper[16][3],
											const unsigned int firstIndex,
											const domListOfUInts& polyIndexData,
											const MeshMod::MaterialIndex matIndex ) {
	vertexIndices.clear();
	// position and faces first
	for( unsigned int indexNum = 0; indexNum < numIndices;++indexNum ) {
		// first up is always position
		MeshMod::VertexIndex posIndex = (MeshMod::VertexIndex) polyIndexData[ (firstIndex + indexNum) * (numVertexExtraAttribs+1) ];
		vertexIndices.push_back( posIndex );
	}
	MeshMod::FaceIndex faceIndex = mesh->addPolygon( vertexIndices );
	(*faceMatEle)[ faceIndex ] = matIndex;

	for( unsigned int indexNum = 0; indexNum < numIndices;++indexNum ) {
		MeshMod::VertexIndex posIndex = (MeshMod::VertexIndex) polyIndexData[ (firstIndex + indexNum) * (numVertexExtraAttribs+1) ];
		for( unsigned int j=0;j < numVertexExtraAttribs;++j ) {
			MeshMod::VertexIndex extraIndex = (MeshMod::VertexIndex) polyIndexData[ ((firstIndex + indexNum) * (numVertexExtraAttribs+1)) + j + 1 ];
			const double* floatList = &((const double*) vertexExtraAttribsHelper[j][2])[ extraIndex * vertexExtraAttribsHelper[j][1] ];

			switch( vertexExtraAttribsHelper[j][0] ) {
				case 1: { // NORMAL 								
					MeshMod::VertexData::Normal normal( (float)floatList[0], (float)floatList[1] , (float)floatList[2] );
					mesh->addVertexAttributeToFace<MeshMod::VertexData::Normal>(	posIndex, faceIndex, *normEle, normal );
					break;
				}
				case 2: { // TEXCOORD
					// todo multiple uv coordinates
					// d3d and collada differ on v direction, my engine uses d3d standard so flip v here.
					MeshMod::VertexData::UV uv( (float)floatList[0], 1.0f - (float) floatList[1] );
					mesh->addVertexAttributeToFace<MeshMod::VertexData::UV>(	posIndex, faceIndex, *uvEle, uv );
					break;
				}
			}
		}
	}
}

void Mesh::AddPolygonsFromVertexSource(  const domMesh* pInMesh ) {

	vertexIndices.clear();
	vertexIndices.reserve(10); // enough for a 10 index polygon

	MeshMod::VertexElementsContainer& vertCon = mesh->getVertexContainer();
	MeshMod::FaceElementsContainer& faceCon = mesh->getFaceContainer();

	normEle = NULL;
	uvEle = NULL;
	faceMatEle = faceCon.getOrAddElements<MeshMod::MaterialFaceElements>();


	const domVertices* vertices = daeSafeCast<domVertices>( pInMesh->getVertices() );
	// the position is probably behind a vertex indrection, dig it out
	if( vertices != NULL ) {
		for (size_t i = 0; i < vertices->getInput_array().getCount(); i++) {
			const domInputLocal* input = vertices->getInput_array()[i];
			xsNMTOKEN sem  = input->getSemantic();

			assert( strcmp( sem, daeString("POSITION") ) == 0 );

			const domURIFragmentType& sourceURI = input->getSource();
			const domSource* source = daeSafeCast<domSource>( sourceURI.getElement() );
			assert( source != NULL );
			const domListOfFloats& sourceFloatData =  source->getFloat_array()->getValue();
			assert( source->getTechnique_common()->getAccessor()->getStride() == 3 );
			for (size_t i = 0; i < source->getFloat_array()->getCount()/3; i++) {
				mesh->addPosition(	(float)sourceFloatData.get(i*3+0), 
									(float)sourceFloatData.get(i*3+1), 
									(float)sourceFloatData.get(i*3+2) );
			}
		}
	}

	// first entry extra vertex attriv type 1 = Normal, 2 = UV;
	// 2nd entry number of float
	// 3rd base pointer to float array
	// repeat upto 16 types
	uintptr_t vertexExtraAttribsHelper[16][3];
	uint32_t numVertexExtraAttribs = 0;

	// triangle lists
	for (size_t triListCount = 0; triListCount  < pInMesh->getTriangles_array().getCount(); triListCount ++) {
		const domTriangles* triList = pInMesh->getTriangles_array()[triListCount ];

		numVertexExtraAttribs = 0;

		// create a material mapping index
		MeshMod::MaterialIndex matIndex = mesh->findMaterial( triList->getMaterial() );
		if( matIndex == MeshMod::GOM_INVALID_INDEX ) {
			MeshMod::MaterialElementsContainer& matCon = mesh->getMaterialContainer();
			MeshMod::NameMaterialElements* nameEle = matCon.getOrAddElements<MeshMod::NameMaterialElements>();
			nameEle->push_back( MeshMod::MaterialData::Name( triList->getMaterial() ) );
			matIndex = nameEle->size() - 1;
		}
		
		// get extra vertex attributes
		for( size_t j = 0; j < triList->getInput_array().getCount(); ++j ) {
			const domInputLocalOffset* inputOffset = triList->getInput_array()[j];
			numVertexExtraAttribs += ExtractSourceInput( inputOffset, vertexExtraAttribsHelper[numVertexExtraAttribs] );
			assert( numVertexExtraAttribs < 16 );
		}

		// now go around each triangle, add triangle and add other vertex attrib (GO handles splits and seams automatically)
		const domListOfUInts& triIndexData = triList->getP()->getValue();
		unsigned int firstIndex = 0;
		for (size_t i = 0; i < triList->getCount();++i) {

			ExtractFaceAndVertexExtraData( 3, numVertexExtraAttribs, vertexExtraAttribsHelper, firstIndex, triIndexData, matIndex );
			firstIndex += 3;
		}
	}

	// polygon lists
	for (size_t polyListCount = 0; polyListCount < pInMesh->getPolylist_array().getCount(); polyListCount++) {
		const domPolylist* polyList = pInMesh->getPolylist_array()[polyListCount];

		numVertexExtraAttribs = 0;

		// create a material mapping index
		MeshMod::MaterialIndex matIndex = mesh->findMaterial( polyList->getMaterial() );
		if( matIndex == MeshMod::GOM_INVALID_INDEX ) {
			MeshMod::MaterialElementsContainer& matCon = mesh->getMaterialContainer();
			MeshMod::NameMaterialElements* nameEle = matCon.getOrAddElements<MeshMod::NameMaterialElements>();
			nameEle->push_back( MeshMod::MaterialData::Name( polyList->getMaterial() ) );
			matIndex = nameEle->size() - 1;
		}
		
		// extra vertex attributes
		for (size_t j=0;j < polyList->getInput_array().getCount(); j++ ) {
			const domInputLocalOffset* inputOffset = polyList->getInput_array()[j];
			numVertexExtraAttribs += ExtractSourceInput( inputOffset, vertexExtraAttribsHelper[numVertexExtraAttribs] );
			assert( numVertexExtraAttribs <16 );

		}

		// now go around each polygon, add polygon and add other vertex attrib (GO handles splits and seams automatically)
		const domListOfUInts& vertexCountData = polyList->getVcount()->getValue();
		const domListOfUInts& polyIndexData = polyList->getP()->getValue();
		unsigned int firstIndex = 0;
		for (size_t i = 0; i < vertexCountData.getCount();++i) {
			unsigned int numIndices = (unsigned int) vertexCountData[i];

			ExtractFaceAndVertexExtraData( numIndices, numVertexExtraAttribs, vertexExtraAttribsHelper, firstIndex, polyIndexData, matIndex );
			firstIndex += numIndices;
		}
	}
}


Mesh::Mesh(domGeometry& geom) : 
	mesh( CORE_NEW MeshMod::Mesh( geom.getID() ) ),
	mtl( NULL ) {
	// Parse the <geometry> element, extract vertex data, etc

	domMesh* daeMesh = daeSafeCast<domMesh>( geom.getMesh() );

	AddPolygonsFromVertexSource( daeMesh );
}

Node::Node(domNode& node) {
	Math::Quaternion nodeQuat = Math::IdentityQuat();
	// this only works for simple transforms of the type T,Rz,Ry,Rx,S
	for( unsigned int i=0;i < node.getContents().getCount();++i ) {
		daeElement* element = node.getContents()[i];
		switch( element->getElementType() ){
			case COLLADA_TYPE::TRANSLATE: {
					domTranslate* translate = daeSafeCast<domTranslate>(element);
					domFloat3& v = translate->getValue();
					this->m_Transform.m_Position = Math::Vector3( (float)v[0], (float)v[1], (float)v[2] );
				}
				break;
			case COLLADA_TYPE::ROTATE: {
					domRotate* rotate = daeSafeCast<domRotate>(element);
					domFloat4& v = rotate->getValue();
					nodeQuat *= Math::CreateRotationQuat( Math::Vector3( (float)v[0], (float)v[1], (float)v[2]), Math::degree_to_radian<float>() * (float)v[3] );
				}
				break;
			case COLLADA_TYPE::SCALE: {
					domScale* scale = daeSafeCast<domScale>(element);
					domFloat3& v = scale->getValue();
					this->m_Transform.m_Scale = Math::Vector3((float)v[0], (float)v[1], (float)v[2] );
				}
				break;
		};
	}
	this->m_Transform.m_Orientation = nodeQuat;
	this->m_name = node.getSid();

	bool hasMesh = false;

	// Recursively convert all child nodes. First iterate over the <node> elements.
	for (size_t i = 0; i < node.getNode_array().getCount(); i++) {
		addChild( MeshMod::SceneNodePtr( &lookup<Node, domNode>(*node.getNode_array()[i]) ) );
	}

	// Then iterate over the <instance_node> elements.
	for (size_t i = 0; i < node.getInstance_node_array().getCount(); i++) {
		domNode* child = daeSafeCast<domNode>(
			node.getInstance_node_array()[i]->getUrl().getElement());
			addChild( MeshMod::SceneNodePtr( &lookup<Node, domNode>(*child) ) );	
	}

	// Iterate over all the <instance_geometry> elements
	for (size_t i = 0; i < node.getInstance_geometry_array().getCount(); i++) {
		domInstance_geometry* instanceGeom = node.getInstance_geometry_array()[i];
		domGeometry* geom = daeSafeCast<domGeometry>(instanceGeom->getUrl().getElement());

		// Now convert the geometry, add the result to our list of meshes, and assign
		// the mesh a material.
		Mesh* mesh = &lookup<Mesh, domGeometry>(*geom);

		for (size_t j = 0; j < instanceGeom->getBind_material()->getTechnique_common()->getInstance_material_array().getCount(); j++) {

			const domInstance_material* instanceMtl = instanceGeom->getBind_material()->getTechnique_common()->getInstance_material_array()[j];
			// get an instancematerial, 
			if(  instanceMtl && mesh->mesh->findMaterial( instanceMtl->getSymbol() ) != MeshMod::GOM_INVALID_INDEX ) {
				// then gets its parent and scan for tall the instance materials
				// check the names match names added during the mesh phase, and if so
				// fill each one in.
				domMaterial* mtl = daeSafeCast<domMaterial>(instanceMtl->getTarget().getElement());
				Material& convertedMtl = lookup<Material, domMaterial>(*mtl);
				convertedMtl.addMesh( instanceMtl, mesh );
			}
		}
		addObject( MeshMod::SceneObjectPtr( mesh->mesh ) );
		hasMesh = true;
	}
	// Iterate over all the <instance_controller> elements
	for (size_t i = 0; i < node.getInstance_controller_array().getCount(); i++) {
		domInstance_controller* instanceCont = node.getInstance_controller_array()[i];

		// get controller url (the actual mesh for joints at least)
		domController* cont = daeSafeCast<domController>(instanceCont->getUrl().getElement());

		domGeometry* geom = daeSafeCast<domGeometry>(cont->getSkin()->getSource().getElement());

		// Now convert the geometry, add the result to our list of meshes, and assign
		// the mesh a material.
		Mesh* mesh = &lookup<Mesh, domGeometry>(*geom);

		for (size_t j = 0; j < instanceCont->getBind_material()->getTechnique_common()->getInstance_material_array().getCount(); j++) {

			const domInstance_material* instanceMtl = instanceCont->getBind_material()->getTechnique_common()->getInstance_material_array()[j];
			// get an instancematerial, 
			if(  instanceMtl && mesh->mesh->findMaterial( instanceMtl->getSymbol() ) != MeshMod::GOM_INVALID_INDEX ) {
				// then gets its parent and scan for all the instance materials
				// check the names match names added during the mesh phase, and if so
				// fill each one in.
				domMaterial* mtl = daeSafeCast<domMaterial>(instanceMtl->getTarget().getElement());
				Material& convertedMtl = lookup<Material, domMaterial>(*mtl);
				convertedMtl.addMesh( instanceMtl, mesh );
			}
		}
		addObject( MeshMod::SceneObjectPtr( mesh->mesh ) );
		hasMesh = true;

/*
		domInstance_controller::domSkeleton* skele = NULL;
		// multiple skeletons not handled
		for (size_t j = 0; j < instanceCont->getSkeleton_array().getCount(); j++) {
			skele = instanceCont->getSkeleton_array()[j];
			// currently break after first skeleton
			break;
		}*/
	}

	// Joints are specific collada joints types and are just skeletal nodes
	// a node without any objects is a node
	// nodes with a single object will be classed based on that (unless its not got a specific type, then it will also be a node)


	switch( node.getType() ) {
	case NODETYPE_JOINT:
		this->m_type = "Joint";
		break;
	default:
	case NODETYPE_NODE:
		if( hasMesh == false ) {
			this->m_type = "Node";
		} else {
			this->m_type = "Mesh";
		}
		break;
	}
}


MeshMod::SceneNodePtr ColladaVisualScene2GO( const domVisual_scene* visualScene ) {

	s_veaLookup[ "POSITION" ] = 0;
	s_veaLookup[ "NORMAL" ] = 1;
	s_veaLookup[ "TEXCOORD" ] = 2;

	MeshMod::SceneNodePtr root = MeshMod::SceneNodePtr( CORE_NEW MeshMod::SceneNode() );

	// Now covert all the <node>s in the <visual_scene>. This is a recursive process,
	// so any child nodes will also be converted.
	const domNode_Array& nodes = visualScene->getNode_array();
	for (size_t i = 0; i < nodes.getCount(); i++) {
		root->addChild( MeshMod::SceneNodePtr( &lookup<Node, domNode>(*nodes[i]) ) );
	}
	return root;
}

void FreeCollada( DAE& dae ) {

//	freeConversionObjects<Node, domNode>( dae );
//	freeConversionObjects<Material, domMaterial>( dae );
//	freeConversionObjects<Mesh, domGeometry>( dae );
	dae.clear();
}

#endif // TODO collada import