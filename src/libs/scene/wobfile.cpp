//!-----------------------------------------------------
//!
//! \file wobfile.cpp
//! functionality for loading wobfiles, the actual saving
//! can be found within the convertmesh project
//!
//!-----------------------------------------------------

#include "scene.h"
#include <iostream>
#include <fstream>
#include "wobfile.h"
#include "core/file_path.h"

namespace Scene {

	std::shared_ptr<WobFileHeader> WobLoad( const char* pFilename ) {
		using namespace Core;
		WobFileHeader header;

		Core::FilePath path( pFilename );
		path = path.ReplaceExtension( ".wob" );

		std::ifstream inStream( path.value().c_str(), std::ifstream::binary );
		inStream.read( (char*)&header, sizeof(header) );
		if( !inStream.good() ) {
			LOG(INFO) << "Wob File " << pFilename << " not found\n";
			assert( false );
		}

		if( header.uiMagic == WobType && header.uiVersion == WobVersion ) {
			// we now allocate the main block and header which is what we
			// pass back to the callee
			std::shared_ptr<WobFileHeader> spHeader( (WobFileHeader*) CORE_NEW char[sizeof(header) + header.ls.uiSizeOfMainBlock] ); 
			WobFileHeader* pHeader = spHeader.get();
			memcpy( pHeader, &header, sizeof(header) );

			// load the main block (materials and parameters string table)
			inStream.read( (char*)(pHeader + 1), pHeader->ls.uiSizeOfMainBlock );
			// load the discardable memory block
			char* pDiscard = CORE_NEW char[pHeader->ls.uiSizeOfDiscardBlock];
			inStream.read( pDiscard, pHeader->ls.uiSizeOfDiscardBlock );
			pHeader->pDiscardable.p = pDiscard;

			// fixup header
			pHeader->pMaterials.p = fixupPointer<WobMaterial>( pHeader, pHeader->pMaterials.o.l );

			// fixup materials
			for( uint32_t i=0;i < pHeader->uiNumMaterials;i++) {
				WobMaterial* pMaterial = pHeader->pMaterials.p+i;
				pMaterial->pName.p = fixupPointer<const char>( pHeader, pMaterial->pName.o.l );
				pMaterial->pShader.p = fixupPointer<const char>( pHeader, pMaterial->pShader.o.l );
				pMaterial->pElements.p = fixupPointer<WobVertexElement>( pHeader, pMaterial->pElements.o.l );
				pMaterial->pParameters.p = fixupPointer<WobMaterialParameter>( pHeader, pMaterial->pParameters.o.l );
				pMaterial->pVertexData.p = fixupPointer<void>( pDiscard, pMaterial->pVertexData.o.l );
				pMaterial->pIndexData.p = fixupPointer<void>( pDiscard, pMaterial->pIndexData.o.l );

				for( uint32_t j=0;j < pMaterial->uiNumParameters;++j ) {
					WobMaterialParameter* pParam = pMaterial->pParameters.p+j;
					pParam->pName.p = fixupPointer<const char>( pHeader, pParam->pName.o.l );
					pParam->pData.p = fixupPointer<void>( pHeader, pParam->pData.o.l );
				}
			}

			return spHeader;
		} else {
			LOG(ERROR) << "Wob File " << pFilename << " has failed to load\n";
			if( header.uiMagic != WobType ) {
				LOG(ERROR) << "This is not a wob file (Magic num failed\n)\n" ;
			}
			if(header.uiVersion != WobVersion) {
				LOG(ERROR) << "This wob file is the wrong version\n";
			}
			assert( false );
		}

		return std::shared_ptr<WobFileHeader>();
	}
} // end namespace Scene
