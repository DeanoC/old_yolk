#pragma once
//!-----------------------------------------------------
//!
//! \file hierfile.h
//! contains a hierachy (node tree) used to load
//! collection of meshes and objects into the engine.
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_HIERFILE_H_)
#define YOLK_SCENE_HIERFILE_H_

#include "core/resources.h"

#define FOP( x, y ) CORE_ALIGN(8) union { x p; struct { uint32_t h; uint32_t l; } o; } y

namespace Scene {	
	// forward decl
	struct HierarchyTree;

	static const uint32_t HierType = RESOURCE_NAME('H','I','E','R');
	static const uint8_t HierVersion = 2;

	enum HierarchyNodeType {
		HNT_NODE		= 0, //!< general unspecified node
		HNT_JOINT		= 1, //!< a skeleton bone/joint node
		HNT_CAMERA		= 2, //!< node has a camera associated
		HNT_LIGHT		= 3, //!< node has a light associated
		HNT_MESH		= 4, //!< node has a mesh associated
	};

	enum HierarchyFileFlags {
		HFF_ENVIRONMENT = BIT(0), //< File has an associated Environment resource
	};

	struct HierarchyFileHeader {
		uint32_t 	uiMagic;				//!< Should but HIER
		uint16_t 	numNodes;				//!< number of nodes
		uint8_t 	version;				//!< hierVersion
		uint8_t 	flags;					//!< none currently
		uint32_t 	dataBlockSize;	//!< size of nodes + link tree block
	};

	enum HierarchyNodeFlags {
		HNF_PROPERTIES = BIT(0), //< node has an associated properties resource
	};

	struct HierarchyNode {
		uint16_t	type;
		uint16_t	flags;

		float		pos[3];
		float		quat[4];
		float		scale[3];

		union {
			// note union must be of pointer types (its fixed up on load)
			FOP( const char*, meshName );
		};
		FOP( HierarchyTree*,	children );
		FOP( const char*,		nodeName );
	};

	struct HierarchyTree {
		uint32_t	numChildren;
		// followed by numChildren HierarchyNode indices
	};

	//! Loads a hie file
	std::shared_ptr<HierarchyFileHeader> HierLoad( const char* pFilename );

	// resource wrarpper for Hier loading
	struct HierResource : public Core::Resource<HierType> {
		struct CreationStruct {};
		struct LoadStruct {};
		std::shared_ptr<HierarchyFileHeader>	header;
	};

	typedef std::shared_ptr<HierResource> HierResourcePtr;
	//! A wob file resource handle typedef
	typedef const Core::ResourceHandle<HierType, HierResource> HierResourceHandle;
	typedef HierResourceHandle* HierResourceHandlePtr;

} // end namespace Scene

#undef FOP

#endif