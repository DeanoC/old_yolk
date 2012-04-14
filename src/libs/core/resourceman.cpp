//!-----------------------------------------------------
//!
//! \file resourceman.cpp
//! a unified resource management system with live loading
//! functionality
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "resourceman.h"
#if PLATFORM == WINDOWS
#	include "platform_windows/dirwatch.h"
#endif
#include <sstream>
#include "coreresources.h"

namespace
{
Core::string GetResourceTypeAsString( uint32_t type ) {
	// get text version of the type number
	// resource types are little endian 4 char strings
	char typeName[5];
#if CPU_ENDIANESS == CPU_LITTLE_ENDIAN
	typeName[3] = (char)((type & 0xFF000000) >> 24);
	typeName[2] = (char)((type & 0x00FF0000) >> 16);
	typeName[1] = (char)((type & 0x0000FF00) >> 8);
	typeName[0] = (char)((type & 0x000000FF) >> 0);
#else
	typeName[0] = (char)((type & 0xFF000000) >> 24);
	typeName[1] = (char)((type & 0x00FF0000) >> 16);
	typeName[2] = (char)((type & 0x0000FF00) >> 8);
	typeName[3] = (char)((type & 0x000000FF) >> 0);
#endif
	typeName[4] = 0;
	return Core::string( typeName );
}
} // end anon namespace

namespace Core
{

struct ResourceData 
{

	Core::scoped_array<const char>				m_ResourceName;
	Core::scoped_array<const char>				m_ResourceData;
	ResourceHandleBase*							m_spHandle;
	Core::shared_ptr<ResourceBase>				m_spResource;
	uint32_t									m_iRefCount;
	RESOURCE_FLAGS								m_Flags;
	Core::string								cacheName;

	ResourceData(	const char* pName, 
					const char* pData, 
					RESOURCE_FLAGS flags, 
					ResourceHandleBase* pBase ) {
		m_ResourceName.reset( pName );
		m_ResourceData.reset( pData );
		m_spHandle = pBase ;
		m_iRefCount = 1;
		m_Flags = flags;
	}
};

struct ResourceTypeData {
	ResourceMan::CreateResourceCallback				m_CreateCallback;
	ResourceMan::DestroyResourceCallback			m_DestroyCallback;
	ResourceMan::ChangeResourceCallback				m_ChangeResourceCallback;
	ResourceMan::DestroyResourceHandleCallback		m_DestroyResourceHandleCallback;
	Core::string									m_ResourceDirectory;
	size_t											m_ResourceHandleSize;	

	ResourceTypeData(){}
	ResourceTypeData(	ResourceMan::CreateResourceCallback& create, 
						ResourceMan::DestroyResourceCallback& destroy,
						ResourceMan::ChangeResourceCallback& change,
						ResourceMan::DestroyResourceHandleCallback destroyHandle,
						const Core::string& dir,
						const size_t _rhSize ) :
		m_CreateCallback( create ), 
		m_DestroyCallback( destroy ), 
		m_ChangeResourceCallback( change ),
		m_DestroyResourceHandleCallback( destroyHandle ),
		m_ResourceDirectory( dir ),
		m_ResourceHandleSize( _rhSize )
	{
	}
};

class ResourceManImpl {
public:

	typedef Core::list<ResourceData*>									ListResourceData;
	typedef Core::unordered_map<ResourceHandleBase*, ResourceData*>		PtrIndex;
	typedef Core::unordered_map<Core::string, ResourceData*>			CacheIndex;
	typedef Core::unordered_map<uint32_t, ResourceTypeData>				ResourceTypeMap;

	ListResourceData	m_ListResourceHandlePtrs;
	PtrIndex			m_ResourceHandleBaseMap;
	CacheIndex			m_CacheMap;
	ResourceTypeMap		m_ResourceTypeMap;

	~ResourceManImpl() {
		if(m_ListResourceHandlePtrs.size() > 0) {
			LOG(INFO) << "Unreleased Resources, these will cause memory leaks :\n";
		}
		ListResourceData::iterator rdIt = m_ListResourceHandlePtrs.begin();
		while( rdIt != m_ListResourceHandlePtrs.end() ) {
			uint32_t type = (*rdIt)->m_spHandle->GetType();
			Core::string typeName = GetResourceTypeAsString( type );
			LOG(INFO) << "Type : " << typeName.c_str() << "(" << type << ")" << 
						" - Name : " << (*rdIt)->m_ResourceName.get() << "\n";
			++rdIt;
		}
	}
};

ResourceMan::ResourceMan() :
	m_impl( *(CORE_NEW ResourceManImpl()) ) {
}

ResourceMan::~ResourceMan() {
	CORE_DELETE &m_impl;
}

const ResourceHandleBase* ResourceMan::ImplOpenResource( const char* pName, const void* pData, size_t sizeofData, uint32_t type, RESOURCE_FLAGS flags ) {
	CORE_ASSERT( (m_impl.m_ResourceTypeMap.find( type ) != m_impl.m_ResourceTypeMap.end() ) &&
			"Resource type not registered" );
	ResourceTypeData& rtd = m_impl.m_ResourceTypeMap[ type ];

	Core::string actualName;
	if( flags & RMRF_LOADOFFDISK ) {
		actualName = rtd.m_ResourceDirectory + pName;
	} else {
		actualName = pName;
	}

	Core::string cacheName;
	Core::stringstream flagString;
	flagString << (flags & ~RMRF_PRELOAD); // used to unique name and flags together remove some we don't care about
	cacheName = actualName + flagString.str();

	ResourceManImpl::CacheIndex::const_iterator cacheIt = m_impl.m_CacheMap.find( cacheName );

	if( cacheIt == m_impl.m_CacheMap.end() ) {
		char *pSafeData = NULL;
		if( pData != NULL ) {
			pSafeData = CORE_NEW_ARRAY char[ sizeofData ]; // will be owned ResourceData
			memcpy(pSafeData, pData, sizeofData);
		}
		char *pSafeName = CORE_NEW_ARRAY char[ actualName.size()+ 1 ]; // will be owned ResourceData
		strcpy(pSafeName, actualName.c_str() );

		uint8_t* baseMem = CORE_NEW_ARRAY uint8_t[ rtd.m_ResourceHandleSize ];
		memset( baseMem, 0, rtd.m_ResourceHandleSize );
		ResourceHandleBase* pBase = CORE_PLACEMENT_NEW(baseMem) ResourceHandleBase( type );
		m_impl.m_ListResourceHandlePtrs.push_back( CORE_NEW ResourceData( pSafeName, pSafeData, flags, pBase ) );
		m_impl.m_ResourceHandleBaseMap[ pBase ] = m_impl.m_ListResourceHandlePtrs.back();

		if( !(flags & RMRF_DONTCACHE) ) {
			m_impl.m_CacheMap[ cacheName ] = m_impl.m_ListResourceHandlePtrs.back();
			m_impl.m_CacheMap[ cacheName ]->cacheName = cacheName;
		}

		return pBase;
	} else {
		ResourceData* pRD = (*cacheIt).second;
		CORE_ASSERT( pRD->m_spHandle->m_Type == type );
		pRD->m_iRefCount++;
		return pRD->m_spHandle;
	}
}

void ResourceMan::ImplCloseResource( ResourceHandleBase* pHandle ) {
	if( m_impl.m_ResourceHandleBaseMap.find( pHandle ) != m_impl.m_ResourceHandleBaseMap.end() ) {
		ResourceData* pRD = m_impl.m_ResourceHandleBaseMap[ pHandle ];
		CORE_ASSERT( pHandle == pRD->m_spHandle );
		--pRD->m_iRefCount;
		if( pRD->m_iRefCount == 0 ) {
			ResourceTypeData& rtd = m_impl.m_ResourceTypeMap[ pRD->m_spHandle->m_Type ];
			if( pRD->m_spResource ) {
				// the callback does the reset DO NOT do it yourself
				rtd.m_DestroyCallback( pRD->m_spResource );
			}

			// cache entry are actually optional so check for valitity before erasing
			ResourceManImpl::CacheIndex::iterator cacheIt = m_impl.m_CacheMap.find( pRD->cacheName );
			if( cacheIt != m_impl.m_CacheMap.end() ) {
				m_impl.m_CacheMap.erase( cacheIt );
			}

			ResourceManImpl::PtrIndex::iterator rhbIt = m_impl.m_ResourceHandleBaseMap.find( pRD->m_spHandle );
			ResourceManImpl::ListResourceData::iterator lrdIt = Core::find( m_impl.m_ListResourceHandlePtrs.begin(),m_impl.m_ListResourceHandlePtrs.end(), pRD );
			m_impl.m_ResourceHandleBaseMap.erase( rhbIt );
			m_impl.m_ListResourceHandlePtrs.erase( lrdIt );

			
			rtd.m_DestroyResourceHandleCallback( pRD->m_spHandle );
			CORE_DELETE pRD;

		}
	} else {
		CORE_ASSERT( false && "ResourceHandle does not exist\n" );
	}
}
void ResourceMan::ImplFlushResource( const char* pName, uint32_t type, RESOURCE_FLAGS flags ) {
	CORE_ASSERT( (m_impl.m_ResourceTypeMap.find( type ) != m_impl.m_ResourceTypeMap.end() ) &&
			"Resource type not registered" );
	ResourceTypeData& rtd = m_impl.m_ResourceTypeMap[ type ];


	Core::string actualName;
	if( flags & RMRF_LOADOFFDISK ) {
		actualName = rtd.m_ResourceDirectory + pName;
	} else {
		actualName = pName;
	}

	Core::string cacheName;
	Core::stringstream flagString;
	flagString << (flags & ~RMRF_PRELOAD); // used to unique name and flags together remove some we don't care about
	cacheName = actualName + flagString.str();

	ResourceManImpl::CacheIndex::iterator cacheIt = m_impl.m_CacheMap.find( cacheName );

	if( cacheIt != m_impl.m_CacheMap.end() ) {
		m_impl.m_CacheMap.erase( cacheIt );
	}
}

static void DefaultDestroyResourceHandleCallback( ResourceHandleBase* handle ) {
	CORE_DELETE handle;
}

void ResourceMan::RegisterResourceType (	uint32_t type, 
											CreateResourceCallback pCreate, 
											DestroyResourceCallback pDestroy, 
											const size_t resourceHandleSize, 
											DestroyResourceHandleCallback pDestroyHandle,							
											ChangeResourceCallback pChange,
											const Core::string dir ) {
	CORE_ASSERT( m_impl.m_ResourceTypeMap.find(type) == m_impl.m_ResourceTypeMap.end() );
	if( pDestroyHandle == NULL ) {
		pDestroyHandle = &DefaultDestroyResourceHandleCallback;
	}

//	Log << "Registering Resource Type : " << GetResourceTypeAsString(type) << " (" << type << ")" << "\n";
	m_impl.m_ResourceTypeMap[ type ] = ResourceTypeData( pCreate, pDestroy, pChange, pDestroyHandle, dir, resourceHandleSize );
}

Core::shared_ptr<ResourceBase> ResourceMan::ImplAcquireResource( ResourceHandleBase* pHandle ) {
	ResourceData* pRD = m_impl.m_ResourceHandleBaseMap[ pHandle ];
	ResourceTypeData& rtd = m_impl.m_ResourceTypeMap[ pHandle->m_Type ];
#if defined( LOG_RESOURCE_ACQUIRE )
	if( pRD->m_Flags & RMRF_LOADOFFDISK ) {
		Log <<	((pRD->m_Flags & RMRF_PRELOAD)? "Preloading " : "Acquiring ") << GetResourceTypeAsString( pHandle->m_type ) << 
				" resource off disk: " << pRD->m_ResourceName.get() <<"\n";
	} else {
		Log << "Acquiring in memory resource: " << pRD->m_ResourceName.get() <<"\n";
	}
#endif
	pRD->m_spResource = rtd.m_CreateCallback( pHandle, pRD->m_Flags, pRD->m_ResourceName.get(), pRD->m_ResourceData.get() );
	if( pRD->m_spResource != NULL ) {
		Core::weak_ptr<ResourceBase> tmp( pRD->m_spResource );
		pHandle->m_wpResourceBase.swap( tmp );
	}
	pRD->m_Flags = (RESOURCE_FLAGS) (pRD->m_Flags & ~(RMRF_PRELOAD ));

	return pRD->m_spResource;
}

void ResourceMan::InternalProcessManifest( uint16_t numEntries, ManifestEntry* entries ) {
	for( uint16_t i=0; i < numEntries; ++i ) {
		uint32_t type = entries[i].type;
		// fixup header
		entries[i].name = fixupPointer<const char>( entries, entries[i].name );

		const char* pFileName = entries[i].name;
		entries[i].handle = ImplOpenResource( pFileName, NULL, 0, type, (RESOURCE_FLAGS)(RMRF_LOADOFFDISK | RMRF_PRELOAD) );
	}
}

void ResourceMan::InternalCloseManifest( uint16_t numEntries, ManifestEntry* entries ) {
	for( uint16_t i=0; i < numEntries; ++i ) {
		ImplCloseResource( (Core::ResourceHandleBase*) entries[i].handle );
		entries[i].handle = NULL;
	}
}

}
