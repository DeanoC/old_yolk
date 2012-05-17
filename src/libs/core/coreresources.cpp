//!-----------------------------------------------------
//!
//! \file coreresources.cpp
//! resource for the core systems
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "resourceman.h"
#include "coreresources.h"
#include "file_path.h"

namespace 
{
//! Callback from the resource manager to create a text resource
std::shared_ptr<Core::ResourceBase> TextCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData )
{
	using namespace Core;

	std::shared_ptr<TextResource> pResource( CORE_NEW TextResource );

	if( flags & RMRF_LOADOFFDISK ) {
		FILE* fh = fopen( pName, "rt" );
		CORE_ASSERT( fh != 0 );
		fseek( fh, 0, SEEK_END);
		long end = ftell(fh);
		fseek( fh, 0, SEEK_SET);
		long start = ftell(fh);
		pResource->m_saText.reset( CORE_NEW_ARRAY char[ (end - start)+1] );
		char* pText = pResource->m_saText.get();
		size_t count = fread( pText, 1, end-start, fh );
		pText[ count ] = 0;

		fclose( fh );
	} else {
		// allocate enough room for the string passed in and copy it
		pResource->m_saText.reset( CORE_NEW_ARRAY char[ strlen( (char*)pData)+1 ] );
		strcpy( pResource->m_saText.get(), (char*)pData );
		pResource->m_saText.get()[ strlen((char*)pData) ] = 0;
	}

	return std::shared_ptr<ResourceBase>(pResource);
}

//! Callback from the resource manager to create a text resource
std::shared_ptr<Core::ResourceBase> ManifestCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData ) {
	using namespace Core;
	std::shared_ptr<ManifestResource> pResource( CORE_NEW ManifestResource );

	if( flags & RMRF_LOADOFFDISK ) {
		uint32_t magic = 0;
		uint16_t version = 0;

		Core::FilePath path( pName );
		path = path.ReplaceExtension( ".man" );
		const char* pManPath = path.value().c_str();

		std::ifstream inStream( pManPath, std::ifstream::binary );
		inStream.read( (char*)&magic, sizeof(magic) );
		if( !inStream.good() ) {
			LOG(FATAL) << "Manifest " << path.value() << " not found\n";
		}
		inStream.read( (char*)&version, sizeof(version) );
		if( magic == Core::ManifestType && version == 1 ) {
			inStream.read( (char*)&pResource->numEntries, sizeof(pResource->numEntries) );
			uint32_t size;
			inStream.read( (char*)&size, sizeof(size) );
			char* rawArray = CORE_NEW_ARRAY char[ size ];
			inStream.read( rawArray, size );

			pResource->entries.reset( (ManifestEntry*) rawArray );
			ResourceMan::get()->internalProcessManifest( pResource->numEntries, pResource->entries.get() );

		} else {
			LOG(FATAL) << "Manifest " << pData << " Invalid\n";
		}
	} else {
		CORE_ASSERT( false );
	}

	return std::shared_ptr<ResourceBase>(pResource);
}

void ManifestResourceDestroyer( std::shared_ptr<Core::ResourceBase>& spBase ) {
	using namespace Core;

	std::shared_ptr<ManifestResource> spActual = std::static_pointer_cast<ManifestResource>(spBase);
	ResourceMan::get()->internalCloseManifest( spActual->numEntries, spActual->entries.get() );

	spBase.reset(); // the order of these two reset is vital for correct
	spActual.reset(); // destruction
}

} // end anon namespace

namespace Core
{
	void InstallResourceTypes() {
		ResourceMan::get()->registerResourceType( TextType, TextCreateResource, &SimpleResourceDestroyer<TextResource>, sizeof(TextResourceHandle) );
		ResourceMan::get()->registerResourceType( ManifestType, ManifestCreateResource, &ManifestResourceDestroyer, sizeof(ManifestResourceHandle), 0, 0, "Manifests/" );
	}
}
