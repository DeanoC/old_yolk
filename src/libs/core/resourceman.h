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

	//! Internal AcquireResource used by ResourceHandle
	template<uint32_t type>
	std::shared_ptr<Resource<type> > internalAcquireResource( const ResourceHandle<type>* pHandle );

	void internalProcessManifest( uint16_t numEntries, struct ManifestEntry* entries );
	void internalCloseManifest( uint16_t numEntries, struct ManifestEntry* entries );

private:
	const ResourceHandleBase* implOpenResource(  const char* pName, const void* pData, const size_t sizeofData, uint32_t type, RESOURCE_FLAGS flags );
	void implCloseResource( ResourceHandleBase* pHandle );
	void implFlushResource( const char* pName, uint32_t type, RESOURCE_FLAGS flags );

	std::shared_ptr<ResourceBase> implAcquireResource( ResourceHandleBase* pHandle );

	class ResourceManImpl&	m_impl;
};


template<uint32_t type>
const ResourceHandle<type>* ResourceMan::loadCreateResource( const char* pName, const void* pData, const size_t sizeofData, uint32_t flags ) {
	const ResourceHandleBase* pRHB = implOpenResource( pName, pData, sizeofData, type, (RESOURCE_FLAGS)flags );
	const ResourceHandle<type>* pRH = (const ResourceHandle<type>*) pRHB;
	if( flags & RMRF_PRELOAD ) {
		if( pRH->m_wpResourceBase.expired() ) {
			// note is allowed to return NULL for the shared pointer as its likely the object isn't ready
			internalAcquireResource( pRH );
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
std::shared_ptr<Resource<type> > ResourceMan::internalAcquireResource( const ResourceHandle<type>* pHandle ) {
	return std::static_pointer_cast<Resource<type> >( 
		implAcquireResource( const_cast<ResourceHandleBase*>(static_cast<const ResourceHandleBase*>(pHandle)) ) );
}


//! function that moves from the type safe external interface to
//! the internal unsafe versions that actual do the work. If
//! a resource is already in existence this is a very quick operation.
//! else it takes an indefinite amount of time, to get the resource
template< uint32_t type >
inline std::shared_ptr<Resource<type> > ResourceHandle<type>::baseAcquire() const {
	while( m_wpResourceBase.expired() ) {
		std::shared_ptr<Resource<type> > acquired( 
				ResourceMan::get()->internalAcquireResource( this ) );
		if(acquired != NULL)
			return acquired;
	}

	return std::static_pointer_cast<Resource<type> >( m_wpResourceBase.lock() );
}


}	//namespace Core


#endif
