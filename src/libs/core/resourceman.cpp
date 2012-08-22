//!-----------------------------------------------------
//!
//! \file resourceman.cpp
//! a unified resource management system with live loading
//! functionality
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "resourceman.h"
#include <sstream>
#include "coreresources.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_vector.h"

namespace
{
std::string getResourceTypeAsString( uint32_t type ) {
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
	return std::string( typeName );
}
} // end anon namespace

namespace Core
{

struct ResourceData 
{
	boost::scoped_array<const char>				resourceName;
	boost::scoped_array<const char>				resourceData;
	ResourceHandleBase*							handle;
	std::shared_ptr<ResourceBase>				resource;
	uint32_t									refCount;
	RESOURCE_FLAGS								flags;
	std::string									cacheName;

	ResourceData(	const char* pName, 
					const char* pData, 
					RESOURCE_FLAGS flags_, 
					ResourceHandleBase* pBase ) {
		resourceName.reset( pName );
		resourceData.reset( pData );
		handle = pBase ;
		refCount = 1;
		flags = flags_;
	}
};

struct ResourceTypeData {
	ResourceMan::CreateResourceCallback				createCallback;
	ResourceMan::DestroyResourceCallback			destroyCallback;
	ResourceMan::ChangeResourceCallback				changeResourceCallback;
	ResourceMan::DestroyResourceHandleCallback		destroyResourceHandleCallback;
	std::string										resourceDirectory;
	size_t											resourceHandleSize;	

	ResourceTypeData(){}
	ResourceTypeData(	ResourceMan::CreateResourceCallback& create, 
						ResourceMan::DestroyResourceCallback& destroy,
						ResourceMan::ChangeResourceCallback& change,
						ResourceMan::DestroyResourceHandleCallback destroyHandle,
						const std::string& dir,
						const size_t _rhSize ) :
		createCallback( create ), 
		destroyCallback( destroy ), 
		changeResourceCallback( change ),
		destroyResourceHandleCallback( destroyHandle ),
		resourceDirectory( dir ),
		resourceHandleSize( _rhSize )
	{
	}
};

class ResourceManImpl {
public:

	typedef tbb::concurrent_vector<ResourceData*>								ListResourceData;
	typedef tbb::concurrent_hash_map<ResourceHandleBase*, ResourceData*>		PtrIndex;
	typedef tbb::concurrent_hash_map<std::string, ResourceData*>				CacheIndex;
	typedef tbb::concurrent_vector<ResourceData*>								ListResourceData;
	typedef tbb::concurrent_hash_map<uint32_t, ResourceTypeData>				ResourceTypeMap;
	typedef tbb::concurrent_vector<ResourceMan::AcquirePumpCallback>			AcquirePumps;

	ListResourceData	listResourceHandlePtrs;
	PtrIndex			resourceHandleBaseMap;
	CacheIndex			cacheMap;
	ResourceTypeMap		resourceTypeMap;
	AcquirePumps		acquirePumps;

	~ResourceManImpl() {
		bool anyValidLeft = false;

		if(listResourceHandlePtrs.size() > 0) {
			auto rdIt = listResourceHandlePtrs.cbegin();
			while( rdIt != listResourceHandlePtrs.cend() ) {
				if( (*rdIt) != nullptr && (*rdIt)->handle != nullptr ) {
					anyValidLeft = true;
					break;
				}
				++rdIt;
			}
		}
		if( anyValidLeft ) {
			LOG(INFO) << "Unreleased Resources, these will cause memory leaks :\n";
			auto rdIt = listResourceHandlePtrs.begin();
			while( rdIt != listResourceHandlePtrs.end() ) {
				if( (*rdIt) == nullptr || (*rdIt)->handle == nullptr ) {
					++rdIt;
					continue;
				}

				uint32_t type = (*rdIt)->handle->getType();
				std::string typeName = getResourceTypeAsString( type );
				LOG(INFO) << "Type : " << typeName.c_str() << "(" << type << ")" << 
							" - Name : " << (*rdIt)->resourceName.get() << "\n";
				++rdIt;
			}
		}
	}
};

ResourceMan::ResourceMan() :
	impl( *(CORE_NEW ResourceManImpl()) ) {
}

ResourceMan::~ResourceMan() {
	CORE_DELETE &impl;
}

const ResourceHandleBase* ResourceMan::implOpenResource( const char* pName, const void* pData, size_t sizeofData, uint32_t type, RESOURCE_FLAGS flags ) {
	ResourceManImpl::ResourceTypeMap::const_accessor roRTM;
	if( impl.resourceTypeMap.find( roRTM, type ) == false ) {
		CORE_ASSERT( false && "Resource type not registered" );
		return nullptr;
	}

	const ResourceTypeData& rtd = roRTM->second;

	std::string actualName;
	if( flags & RMRF_LOADOFFDISK ) {
		actualName = rtd.resourceDirectory + pName;
	} else {
		actualName = pName;
	}

	const std::string& cacheName = actualName;

	ResourceManImpl::CacheIndex::const_accessor rcacheIt;
	if( impl.cacheMap.find( rcacheIt, cacheName ) ) {
		ResourceData* pRD = rcacheIt->second;
		CORE_ASSERT( pRD->handle->type == type );
		pRD->refCount++;
		return pRD->handle;
	}

	char *pSafeData = NULL;
	if( pData != NULL ) {
		pSafeData = CORE_NEW_ARRAY char[ sizeofData ]; // will be owned ResourceData
		memcpy(pSafeData, pData, sizeofData);
	}
	uint8_t* baseMem = CORE_NEW_ARRAY uint8_t[ rtd.resourceHandleSize ];
	memset( baseMem, 0, rtd.resourceHandleSize );
	ResourceHandleBase* pBase = CORE_PLACEMENT_NEW(baseMem) ResourceHandleBase( type );

	ResourceManImpl::PtrIndex::accessor acc;
	if( impl.resourceHandleBaseMap.insert( acc, pBase ) ) {	
		char *pSafeName = CORE_NEW_ARRAY char[ actualName.size()+ 1 ]; // will be owned ResourceData
		strcpy(pSafeName, actualName.c_str() );
		auto lb = impl.listResourceHandlePtrs.push_back( CORE_NEW ResourceData( pSafeName, pSafeData, flags, pBase ) );
		acc->second = *lb;
		if( !(flags & RMRF_DONTCACHE) ) {
			ResourceManImpl::CacheIndex::accessor cacheIt;
			if( impl.cacheMap.insert( cacheIt, cacheName ) ) {
				cacheIt->second = *lb;
				cacheIt->second->cacheName = cacheName;
			}
		}

	} else {
		CORE_DELETE_ARRAY( pSafeData );
		CORE_DELETE_ARRAY( baseMem );
		CORE_ASSERT( false && "resourceHandleBaseMap.insert failed" )
	}

	return pBase;
}

void ResourceMan::baseCloseResource( ResourceHandleBase* pHandle ) {
	ResourceManImpl::PtrIndex::accessor acc;
	if( impl.resourceHandleBaseMap.find( acc, pHandle )  ) {
		ResourceData* pRD = acc->second;
		CORE_ASSERT( pHandle == pRD->handle );
		--pRD->refCount;
		if( pRD->refCount == 0 ) {
			ResourceManImpl::ResourceTypeMap::const_accessor roRTM;
			if( impl.resourceTypeMap.find( roRTM, pHandle->type ) == false ) {
				CORE_ASSERT( false && "Resource type not registered" );
			}

			// cache entry are actually optional so check for valitity before erasing
			ResourceManImpl::CacheIndex::accessor cacheIt;
			if( impl.cacheMap.find( cacheIt, pRD->cacheName ) ) {
				impl.cacheMap.erase( cacheIt );
			}

			const ResourceTypeData& rtd = roRTM->second;
			// the callback does the reset DO NOT do it yourself
			if( pRD->resource ) {
				rtd.destroyCallback( pRD->resource );
			}

			// currently the vector just grows, null entries and releasing memory but not actually
			// shriking the vector. This could be done by a stop the world mutex by for now probably okay as only 4/8 byte per resource
			ResourceManImpl::ListResourceData::iterator lrdIt = std::find( impl.listResourceHandlePtrs.begin(),impl.listResourceHandlePtrs.end(), pRD );
			*lrdIt = nullptr;

			impl.resourceHandleBaseMap.erase( acc );
			
			rtd.destroyResourceHandleCallback( pRD->handle );
			pRD->handle = nullptr;
			CORE_DELETE pRD;
				
		}
	} else {
		CORE_ASSERT( false && "TypedResourceHandle does not exist\n" );
	}
}
ResourceHandleBase* ResourceMan::baseCloneResource( ResourceHandleBase* pHandle ) {
	ResourceManImpl::PtrIndex::accessor acc;
	if( impl.resourceHandleBaseMap.find( acc, pHandle )  ) {
		ResourceData* pRD = acc->second;
		CORE_ASSERT( pHandle == pRD->handle );
		++pRD->refCount;
	} else {
		CORE_ASSERT( false && "TypedResourceHandle does not exist\n" );		
	}
	return pHandle;
}

void ResourceMan::implFlushResource( const char* pName, uint32_t type, RESOURCE_FLAGS flags ) {
	ResourceManImpl::ResourceTypeMap::const_accessor roRTM;
	if( impl.resourceTypeMap.find( roRTM, type ) == false ) {
		CORE_ASSERT( false && "Resource type not registered" );
	}

	const ResourceTypeData& rtd = roRTM->second;

	std::string actualName;
	if( flags & RMRF_LOADOFFDISK ) {
		actualName = rtd.resourceDirectory + pName;
	} else {
		actualName = pName;
	}

	const std::string& cacheName = actualName;

	// cache entry are actually optional so check for valitity before erasing
	ResourceManImpl::CacheIndex::accessor cacheIt;
	if( impl.cacheMap.find( cacheIt, cacheName ) ) {
		impl.cacheMap.erase( cacheIt );
	}
}

static void DefaultDestroyResourceHandleCallback( ResourceHandleBase* handle ) {
	CORE_DELETE handle;
}

void ResourceMan::registerResourceType (	uint32_t type, 
											CreateResourceCallback pCreate, 
											DestroyResourceCallback pDestroy, 
											const size_t resourceHandleSize, 
											DestroyResourceHandleCallback pDestroyHandle,							
											ChangeResourceCallback pChange,
											const std::string dir ) {
	ResourceManImpl::ResourceTypeMap::accessor rtmIt;
	if( impl.resourceTypeMap.find( rtmIt, type ) == true ) {
		CORE_ASSERT( false && "Resource type already registered" );
	}

	if( pDestroyHandle == NULL ) {
		pDestroyHandle = &DefaultDestroyResourceHandleCallback;
	}

	if( impl.resourceTypeMap.insert( rtmIt, type ) == true ) {
//	LOG(INFO) << "Registering Resource Type : " << GetResourceTypeAsString(type) << " (" << type << ")" << "\n";
		rtmIt->second = ResourceTypeData( pCreate, pDestroy, pChange, pDestroyHandle, dir, resourceHandleSize );
	}
}
void ResourceMan::registerAcquirePump( AcquirePumpCallback pump ) {
	impl.acquirePumps.push_back( pump );
}
void ResourceMan::internalPumpAcquirePump() {
	for( auto it = impl.acquirePumps.cbegin(); it != impl.acquirePumps.cend(); ++it ) {
		(*it)();
	}
}

std::shared_ptr<ResourceBase> ResourceMan::implAcquireResource( ResourceHandleBase* pHandle ) {
	int alreadyAcquiring = pHandle->acquiring.fetch_add( 1 );	

	if( alreadyAcquiring ) {
		pHandle->acquiring += -1;	
		return std::shared_ptr<ResourceBase>();
	}
	if( auto res = pHandle->resourceBase.lock() ) {
		return res;
	}

	ResourceManImpl::ResourceTypeMap::const_accessor roRTM;
	if( impl.resourceTypeMap.find( roRTM, pHandle->type ) == false ) {
		CORE_ASSERT( false && "Resource type not registered" );
	}
	const ResourceTypeData& rtd = roRTM->second;

	ResourceManImpl::PtrIndex::const_accessor rdIt;
	if( impl.resourceHandleBaseMap.find( rdIt, pHandle ) == false ) {
		CORE_ASSERT( false && "Invalid Resource require" );		
	}
	const ResourceData* pRD = rdIt->second;
	rdIt.release();

	rtd.createCallback( pHandle, pRD->flags, pRD->resourceName.get(), pRD->resourceData.get() );

	if( auto res = pHandle->resourceBase.lock() ) {
		return res;
	} else {
		return std::shared_ptr<ResourceBase>();
	}
}

void ResourceMan::internalAcquireComplete( const ResourceHandleBase* _handle, std::shared_ptr<ResourceBase>& res ) {
	ResourceHandleBase* pHandle = const_cast<ResourceHandleBase*>(_handle);

	if( !res ) {
		pHandle->acquiring += -1;
		return;
	}

	ResourceManImpl::PtrIndex::accessor rdIt;
	if( impl.resourceHandleBaseMap.find( rdIt, pHandle ) == false ) {
		CORE_ASSERT( false && "Invalid Resource require" );
	}

	ResourceData* pRD = rdIt->second;
	pRD->resource = res;
	pRD->flags = (RESOURCE_FLAGS) pRD->flags;

	pHandle->resourceBase = res;

	pHandle->acquiring += -1;	
}

void ResourceMan::internalProcessManifest( uint16_t numEntries, ManifestEntry* entries ) {
	for( uint16_t i=0; i < numEntries; ++i ) {
		uint32_t type = entries[i].type;
		// fixup header
		entries[i].name.p = fixupPointer<const char>( entries, entries[i].name.o.l );

		const char* pFileName = entries[i].name.p;
		entries[i].handle.p = implOpenResource( pFileName, NULL, 0, type, RMRF_LOADOFFDISK );
	}
}

void ResourceMan::internalCloseManifest( uint16_t numEntries, ManifestEntry* entries ) {
	for( uint16_t i=0; i < numEntries; ++i ) {
		baseCloseResource( (Core::ResourceHandleBase*) entries[i].handle.p );
		entries[i].handle.p = NULL;
	}
}

}
