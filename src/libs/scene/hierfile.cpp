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
		inStream.read( (char*)&header, sizeof(HierarchyFileHeader) );
		if( !inStream.good() ) {
			if( inStream.eof() ) {
				LOG(INFO) << "Hie File " << pFilename << " header too small\n";
				return std::shared_ptr<HierarchyFileHeader>();				
			} else {
				LOG(INFO) << "Hie File " << pFilename << " not found\n";
				return std::shared_ptr<HierarchyFileHeader>();				
			}
		}
		if( header.uiMagic == HieType ) {
			if( header.version != HierVersion ) {
				LOG(INFO) << "Hie File " << pFilename << " version incorrect\n";
				return std::shared_ptr<HierarchyFileHeader>();
			}

			// load the nodes
			std::shared_ptr<HierarchyFileHeader> spHeader( (HierarchyFileHeader*) 
				CORE_NEW char[sizeof(HierarchyFileHeader) + header.dataBlockSize ] ); 

			HierarchyFileHeader* pHeader = spHeader.get();
			memcpy( pHeader, &header, sizeof(HierarchyFileHeader) );

			HierarchyNode* onodes = (HierarchyNode*) (pHeader + 1);
			inStream.read( (char*)onodes, header.dataBlockSize );
			HierarchyNode* nodes = (HierarchyNode*) Core::alignTo( (uintptr_t) onodes, 8 );

			for(int i = 0;i < header.numNodes;++i ) {
				if( nodes[i].meshName.o.l != 0 ) {
					// we do mesh name but will also do any other union'ed pointer
					nodes[i].meshName.p = fixupPointer<const char>( onodes, nodes[i].meshName.o.l );
				}
				if( nodes[i].children.o.l != 0 ) {
					nodes[i].children.p = fixupPointer<HierarchyTree>( onodes, nodes[i].children.o.l );
					HierarchyTree* tree = nodes[i].children.p;
					union Tmp { 
						HierarchyNode* p; 
						struct { uint32_t h; uint32_t l; } o; 
					} *child;
					child = (Tmp*)(tree + 1);

					for( unsigned int i = 0; i < tree->numChildren;++i ) {
						child->p = fixupPointer<HierarchyNode>( onodes, child->o.l );
						child++;
					}
				}
				nodes[i].nodeName.p = fixupPointer<const char>( onodes, nodes[i].nodeName.o.l );
			}
			return spHeader;
		}
		return std::shared_ptr<HierarchyFileHeader>();
	}

const void* Hie::internalPreCreate( const char* name, const Hie::CreationInfo * ) {
	auto ret = HierLoad( name );
	if( ret ) {
		auto hie = CORE_NEW Hie();
		hie->header = ret;
		return hie;
	} else {
		return nullptr;
	}
}

} // end namespace Scene