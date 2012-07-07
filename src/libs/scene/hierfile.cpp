//!-----------------------------------------------------
//!
//! \file hierfile.cpp
//! functionality for loading hier files, the actual saving
//! can be found within the convertmesh project
//!
//!-----------------------------------------------------

#include "scene.h"
#include <iostream>
#include <fstream>
#include "core/file_path.h"
#include "hierfile.h"

namespace Scene {

	std::shared_ptr<HierarchyFileHeader>  HierLoad( const char* pFilename ) {
		using namespace Core;
		HierarchyFileHeader header;

		Core::FilePath path( pFilename );
		path = path.ReplaceExtension( ".hie" );

		std::ifstream inStream( path.value().c_str(), std::ifstream::binary );
		inStream.read( (char*)&header, sizeof(header) );
		if( !inStream.good() ) {
			LOG(INFO) << "Hie File " << pFilename << " not found\n";
			return std::shared_ptr<HierarchyFileHeader>();
		}
		if( header.uiMagic == HierType ) {
			// load the nodes
			std::shared_ptr<HierarchyFileHeader> spHeader( (HierarchyFileHeader*) CORE_NEW char[sizeof(header) + header.numNodes * sizeof(HierarchyNode) + header.linkBlockSize ] ); 
			HierarchyFileHeader* pHeader = spHeader.get();
			memcpy( pHeader, &header, sizeof(header) );
			if( pHeader->version != HierVersion ) {
				LOG(INFO) << "Hie File " << pFilename << " version incorrect\n";
				return std::shared_ptr<HierarchyFileHeader>();				
			}

			HierarchyNode* nodes = (HierarchyNode*) (pHeader + 1);
			inStream.read( (char*)nodes, header.numNodes * sizeof(HierarchyNode) );

			// load the links, 1 uint16_t numchild followed by
			uint8_t* links = (uint8_t*)(nodes + header.numNodes);
			inStream.read( (char*)links, header.linkBlockSize );
			pHeader->pLinkBlock.p = links;

			for(int i = 0;i < header.numNodes;++i ) {
				nodes[i].children.p = fixupPointer<HierarchyTree>( links, nodes[i].children.o.l );
				nodes[i].nodeName.p = fixupPointer<const char>( links, nodes[i].nodeName.o.l );
				if( nodes[i].meshName.p != 0 ) {
					// we do mesh name but will also do any other union'ed pointer
					nodes[i].meshName.p = fixupPointer<const char>( links, nodes[i].meshName.o.l );
				}
			}
			return spHeader;
		}
		return std::shared_ptr<HierarchyFileHeader>();
	}
} // end namespace Scene