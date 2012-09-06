#pragma once
//!-----------------------------------------------------
//!
//! \file wobfile.h
//! contains a simple 3D object file format used to load
//! meshes into the engine.
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_WOBFILE_H_)
#define YOLK_SCENE_WOBFILE_H_

#include "core/resources.h"

#include "pipeline.h"

#define FOP( x, y ) CORE_ALIGN(8) union { x p; struct { uint32_t h; uint32_t l; } o; } y


namespace Scene {
	class Renderer;

    //! Resource type idenetifier for Wobs
	static const uint32_t WobType = RESOURCE_NAME('W','O','B','1');

	//! defines a vertex element, consists of a usage and a format
	struct WobVertexElement {
		//! what this vertex type is for
		enum Usage {
			WVTU_POSITION = 0,
			WVTU_NORMAL,
			WVTU_BINORMAL,
			WVTU_TANGENT,
			WVTU_TEXCOORD0,
			WVTU_TEXCOORD1,
			WVTU_TEXCOORD2,
			WVTU_TEXCOORD3,
			WVTU_COLOUR0,
			WVTU_COLOUR1,
			WVTU_BONEINDICES,
			WVTU_BONEWEIGHTS,
		};

		//! what the data type is
		enum Type {
			WVTT_FLOAT3 = 0,
			WVTT_FLOAT2,
			WVTT_BYTEARGB,			//!< D3DCOLOR 8 bit ARGB 
			WVTT_FLOAT1,
			WVTT_FLOAT4,
			WVTT_SHORT2
		};
		uint16_t uiUsage;	//!< what the usage is
		uint16_t uiType;	//!< what the type is

		//! compares to vertex element arrays for equality
		static bool Compare( int numElements, WobVertexElement *lhs, WobVertexElement* rhs ) {
			for( int i=0; i < numElements; ++i ) {
				if( lhs->uiUsage != rhs->uiUsage )
					return false;
				if( lhs->uiType != rhs->uiType )
					return false;

				++lhs;
				++rhs;
			}
			return true;
		}
	};

	struct WobMaterialParameter {
		enum Type {
			WMPT_SCALAR_FLOAT = 0,
			WMPT_VEC2_FLOAT,
			WMPT_VEC3_FLOAT,
			WMPT_VEC4_FLOAT,

			WMPT_SAMPLER_CUBE_MAP,
			WMPT_SAMPLER_1D,
			WMPT_SAMPLER_2D,
			WMPT_SAMPLER_3D,
			
			WMPT_TEXTURE_CUBE_MAP,
			WMPT_TEXTURE_1D,
			WMPT_TEXTURE_2D,
			WMPT_TEXTURE_3D,
		};
		enum Flags {
			WMPF_ANIMATED = (1 << 0),
		};

		uint16_t	uiType;
		uint16_t	uiFlags;

		FOP( const char*, pName );
		FOP( void*,		pData );
	};

	//! flags for wob files
	enum WOB_FLAGS {
		WF_SKINNED = (1 << 0),					//!< this wob has bone weights and matrix indices
		WF_VERTEXMORPHS = (1 << 1)				//!< this wob has vertex delta morphs
	};


	//! A material is a renderable chunk has index and vertex data and
	//! the material parameters
	struct WobMaterial {
		//! material flags
		enum Flags {
			WM_32BIT_INDICES = BIT(0),				//!< index data is 32 bit per entry
			WM_TRANSPARENT = BIT(1),				//!< material is transparent or translucent
		};
		FOP( const char*,		pName );			//!< Name of the material
		FOP( const char*,		pShader );			//!< Name of the shader

		uint8_t					numVertexElements;	//!< Number of vertex elements
		uint8_t					uiNumParameters;	//!< how many material parameters we have
		uint16_t				uiFlags;			//!< material flags (alpha etc.)

		uint32_t				uiNumVertices;		//!< number of vertices for this material
		uint32_t				uiNumIndices;		//!< number of indices for this material

		float					minAABB[3];			//!< minimum coord of AABB (xyz)
		float					maxAABB[3];			//!< maximum coord of AABB (xyz)

		FOP( WobVertexElement*,		pElements );		//!< pointer to start vertex element array
		FOP( WobMaterialParameter*,	pParameters );		//!< pointer to the parameter block
		FOP( void*,					pIndexData );		//!< pointer to the index data

		FOP( void*,				pVertexData );		//!< pointer to the actual vertex data
		FOP( void*,				backEndData );		//!< pointer to a backend specific structure after vertex data has been uploaded
	};

	//! Header of a wob file
	struct WobFileHeader {
		uint32_t uiMagic;				//!< Should but WOB1

		uint16_t uiVersion;				//!< an incrementing version number
		uint16_t uiNumMaterials;		//!< number of different materials used

		uint32_t uiFlags;				//!< WFH_FLAGS for this mesh
		FOP( const char*, pName );		//!< name of this Wob

		float	minAABB[3];				//!< minimum coord of AABB (xyz)
		float	maxAABB[3];				//!< maximum coord of AABB (xyz)

		FOP( WobMaterial*,	pMaterials );		//!< array of material pointers

		union {
			struct {
				uint32_t		uiSizeOfMainBlock;		//!< size of the non discardable block
				uint32_t		uiSizeOfDiscardBlock;	//!< size of the discardable block
			} ls;
			FOP( char*,		pDiscardable );			//!< after load this points to the discard memory, delete[] when you have finished
		};
	};


	static const uint16_t WobVersion = 7;

	//! Loads a wob file
	WobFileHeader* WobLoad( const char* pFilename );

	//! An actual wob resource, has a wob file headers and the materials
	//! the file header is patched to point directly the the materials here
	class Wob : public Core::Resource<WobType> {
	public:
		Wob(){};
		friend class ResourceLoader;
		struct CreationStruct {
			std::shared_ptr<WobFileHeader> header;
		};
		struct CreationInfo {};

		std::shared_ptr<WobFileHeader>							header;
		std::vector<std::unique_ptr<PipelineDataStore>>			pipelineDataStores;
	protected:
		static const void* internalPreCreate( const char* name, const Wob::CreationInfo *loader );
		static Wob* internalCreate( Renderer* renderer, const void* data );
	};

	typedef std::shared_ptr<Wob> WobPtr;
	//! A wob file resource handle typedef
	typedef const Core::ResourceHandle<WobType, Wob> WobHandle;
	typedef WobHandle* WobHandlePtr;
}

#undef FOP

#endif // WIERD_GRAPHICS_WOBFILE_H