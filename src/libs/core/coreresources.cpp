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

//namespace 
//{
//! Callback from the resource manager to create a text resource
void TextCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData )
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
		pResource->text.reset( CORE_NEW_ARRAY char[ (end - start)+1] );
		char* pText = pResource->text.get();
		size_t count = fread( pText, 1, end-start, fh );
		pText[ count ] = 0;

		fclose( fh );
	} else {
		// allocate enough room for the string passed in and copy it
		pResource->text.reset( CORE_NEW_ARRAY char[ strlen( (char*)pData)+1 ] );
		strcpy( pResource->text.get(), (char*)pData );
		pResource->text.get()[ strlen((char*)pData) ] = 0;
	}

	auto res = std::shared_ptr<ResourceBase>(pResource);
	Core::ResourceMan::get()->internalAcquireComplete( handle, res );
}

//! Callback from the resource manager to create a text resource
void ManifestCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData ) {
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

	auto res = std::shared_ptr<ResourceBase>(pResource);
	Core::ResourceMan::get()->internalAcquireComplete( handle, res );
}

void ManifestResourceDestroyer( std::shared_ptr<Core::ResourceBase>& spBase ) {
	using namespace Core;

	std::shared_ptr<ManifestResource> spActual = std::static_pointer_cast<ManifestResource>(spBase);
	ResourceMan::get()->internalCloseManifest( spActual->numEntries, spActual->entries.get() );

	spBase.reset(); // the order of these two reset is vital for correct
	spActual.reset(); // destruction
}

//! Header of a property file	
struct BinPropertyFileHeader {
	uint32_t uiMagic;				//!< Should be PROP

	uint16_t uiVersion;				//!< an incrementing version number
	uint16_t uiNumProperties;		//!< number of different properties
	uint32_t size;					//!< total size
	// padding so that properties start on a 64 bit alignment 
};

//! Callback from the resource manager to create a binary property resource
void BinPropertyCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData ) {
	using namespace Core;
	std::shared_ptr<BinPropertyResource> pResource( CORE_NEW BinPropertyResource );

	if( flags & RMRF_LOADOFFDISK ) {
		uint32_t magic = 0;
		uint16_t version = 0;

		Core::FilePath path( pName );
		path = path.ReplaceExtension( ".prp" );
		const char* pManPath = path.value().c_str();

		std::ifstream inStream( pManPath, std::ifstream::binary );
		inStream.read( (char*)&magic, sizeof(magic) );
		if( !inStream.good() ) {
			LOG(FATAL) << "Binary Propertys - " << path.value() << " not found\n";
		}
		inStream.read( (char*)&version, sizeof(version) );
		if( magic == Core::BinPropertyType && version == 2 ) {
			inStream.read( (char*)&pResource->numEntries, sizeof(pResource->numEntries) );
			uint32_t size;
			inStream.read( (char*)&size, sizeof(size) );
			uint32_t dummy;
			inStream.read( (char*)&dummy, sizeof(dummy) );

			char* rawArray = CORE_NEW_ARRAY char[ size ];
			inStream.read( rawArray, size );
			pResource->totalSize =  size;
			pResource->entries.reset( (BinProperty*) rawArray );
			BinProperty* props = pResource->entries.get();
			
			for( int i = 0;i < pResource->numEntries; ++i ) {
				props[i].name.p = fixupPointer<const char>( props, props[i].name.o.l );
				props[i].data.p = fixupPointer<void>( props, props[i].data.o.l );
			}
		} else {
			LOG(FATAL) << "Binary Propertys " << pData << " Invalid\n";
		}
	} else {
		CORE_ASSERT( false );
	}

	auto res = std::shared_ptr<ResourceBase>(pResource);
	Core::ResourceMan::get()->internalAcquireComplete( handle, res );
}

//} // end anon namespace

namespace Core
{
	void InstallResourceTypes() {
		ResourceMan::get()->registerResourceType( TextType, TextCreateResource, 
		                                &SimpleResourceDestroyer<TextResource>, sizeof(TextResourceHandle) );
		ResourceMan::get()->registerResourceType( ManifestType, ManifestCreateResource, 
		                               	&ManifestResourceDestroyer, sizeof(ManifestResourceHandle), 0, 0, "Manifests/" );
		ResourceMan::get()->registerResourceType( BinPropertyType, BinPropertyCreateResource, 
		                               	&SimpleResourceDestroyer<TextResource>, sizeof(BinPropertyResourceHandle), 0, 0, "Properties/" );

	}
}
