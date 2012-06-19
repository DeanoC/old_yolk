//!-----------------------------------------------------
//!
//! \file resourcemanager.h
//!   Copyright (C) 2004 by Dean Calver                                     
//!   deano@rattie.demon.co.uk                                              
//!
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_RESOURCEMANAGER_H
#define WIERD_CORE_RESOURCEMANAGER_H

#include "resourcebase.h"

namespace Core
{

//!-----------------------------------
//!
//! Resource manager singleton
//! All resources allocation goes through here
//!
//!-----------------------------------
class ResourceMan : public Singleton<ResourceMan> {
public:
	friend class ResourceHandleBase;
	
	ResourceMan();
	~ResourceMan();

	//! called to create a resource with the provided data
	typedef std::shared_ptr<ResourceBase> (*CreateResourceCallback)( const ResourceHandleBase* handle, RESOURCE_FLAGS flags, const char* pName, const void* pData );
	//! called to destory the provided resource
	typedef void (*DestroyResourceCallback)( std::shared_ptr<ResourceBase>& );
	//! called the resource has change, return true to flush
	typedef bool (*ChangeResourceCallback)( std::shared_ptr<ResourceBase>& );
	//! called the resourcehandle is destroyed (on final Close), should call CORE_DELETE on the derived handle type
	typedef void (*DestroyResourceHandleCallback)( ResourceHandleBase* handle );


	//! register callback for a new resource type
	void registerResourceType (		uint32_t type, //!< unique ID for resource
									CreateResourceCallback pCreate,  //!< required callback 
									DestroyResourceCallback pDestroy,//!< required callback 
									const size_t resourceHandleSize, //!< required total size of the handle, will be zero'd on handle creation
									DestroyResourceHandleCallback pDestroyHandle = NULL,//!< optional callback NULL uses default destroy
									ChangeResourceCallback pChange= 0, //!< Optional callback when change occurs
									const std::string dir = ""			//!< Optional directory prefix
									);

	// Load or Create a resource, for load name is filename, data is the resource specific load structure, 
	// for create name is internal name to allow caching or not, data is the resource specific create structure
	template< uint32_t type >
	const ResourceHandle<type>* loadCreateResource( const char* pName, const void* pData, const size_t sizeofData, uint32_t flags );

	//! used when the resource is finished
	template<uint32_t type>
	void closeResource( const ResourceHandle<type>* handle );

	//! flushes a resource fromt he caches
	template<uint32_t type>
	void flushResource( const char* pName, uint32_t flags );


	void internalProcessManifest( uint16_t numEntries, struct ManifestEntry* entries );
	void internalCloseManifest( uint16_t numEntries, struct ManifestEntry* entries );

	void internalAsyncAcquireComplete( const ResourceHandleBase* _handle, std::shared_ptr<ResourceBase>& _resource );

private:
	//! Internal AcquireResource used by ResourceHandle
	template<uint32_t type>
		std::shared_ptr<Resource<type> > internalAcquireResource( const ResourceHandleBase* const pHandle );


	const ResourceHandleBase* implOpenResource(  const char* pName, const void* pData, const size_t sizeofData, uint32_t type, RESOURCE_FLAGS flags );
	void implCloseResource( ResourceHandleBase* pHandle );
	void implFlushResource( const char* pName, uint32_t type, RESOURCE_FLAGS flags );

	std::shared_ptr<ResourceBase> implAcquireResource( ResourceHandleBase* pHandle );

	class ResourceManImpl&	impl;
};


template<uint32_t type>
const ResourceHandle<type>* ResourceMan::loadCreateResource( const char* pName, const void* pData, const size_t sizeofData, uint32_t flags ) {
	const ResourceHandleBase* pRHB = implOpenResource( pName, pData, sizeofData, type, (RESOURCE_FLAGS)flags );
	const ResourceHandle<type>* pRH = (const ResourceHandle<type>*) pRHB;
	if( flags & RMRF_PRELOAD ) {
		if( pRH->resourceBase.expired() ) {
			// note is allowed to return NULL for the shared pointer as its likely the object isn't ready
			internalAcquireResource<type>( pRHB );
		}
	}
	return pRH;
}


template<uint32_t type>
void ResourceMan::closeResource( const ResourceHandle<type>* pHandle ) {
	implCloseResource( const_cast<ResourceHandleBase*>(static_cast<const ResourceHandleBase*>(pHandle)) );
}


template<uint32_t type>
void ResourceMan::flushResource( const char* pName, uint32_t flags ) {
	implFlushResource( pName, type, (RESOURCE_FLAGS)flags );
}


template<uint32_t type>
std::shared_ptr<Resource<type> > ResourceMan::internalAcquireResource( const ResourceHandleBase* const pHandle ) {
	return std::static_pointer_cast<Resource<type> >( 
		implAcquireResource( const_cast<ResourceHandleBase*>(pHandle) ) );
}


//! function that moves from the type safe external interface to
//! the internal unsafe versions that actual do the work. If
//! a resource is already in existence this is a very quick operation.
//! else it takes an indefinite amount of time, to get the resource
template< uint32_t type_ >
inline std::shared_ptr<Resource<type_> > ResourceHandleBase::baseAcquire() const {
	do {
		if( auto res = resourceBase.lock() ) {
			return std::static_pointer_cast<Resource<type_> >( res );
		} else {
			std::shared_ptr<Resource<type_> > acquiredp( 
					ResourceMan::get()->internalAcquireResource<type_>( this ) );
			if( acquiredp )
				return acquiredp;
		}
	} while( true );
}
//! like baseAcquire but doesn't stall, if its not ready it will return a empty
//! shared pointer
template< uint32_t type_ >
inline std::shared_ptr<Resource<type_> > ResourceHandleBase::baseTryAcquire() const {
	if( auto res = resourceBase.lock() ) {
		return std::static_pointer_cast<Resource<type_> >( res );
	} else {
		std::shared_ptr<Resource<type_> > acquiredp( 
				ResourceMan::get()->internalAcquireResource<type_>( this ) );
		if( acquiredp ) {
			return acquiredp;
		} else {
			return std::shared_ptr<Resource<type_> >();
		}
	}
}

}	//namespace Core


#endif
