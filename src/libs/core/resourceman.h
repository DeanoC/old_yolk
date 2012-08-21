//!-----------------------------------------------------
//!
//! \file resourcemanager.h
//!   Copyright (C) 2004 by Dean Calver                                     
//!   deano@rattie.demon.co.uk                                              
//!
//!
//!-----------------------------------------------------

#pragma once

#ifndef YOLK_CORE_RESOURCEMAN_H_
#define YOLK_CORE_RESOURCEMAN_H_

#ifndef YOLK_CORE_RESOURCEBASE_H_
#	include "resourcebase.h"
#endif
#ifndef YOLK_CORE_SINGLETON_H_
#	include "singleton.h"
#endif

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
	typedef void (*CreateResourceCallback)( const ResourceHandleBase* handle, RESOURCE_FLAGS flags, const char* pName, const void* pData );
	//! called to destory the provided resource
	typedef void (*DestroyResourceCallback)( std::shared_ptr<ResourceBase>& );
	//! called the resource has change, return true to flush
	typedef bool (*ChangeResourceCallback)( std::shared_ptr<ResourceBase>& );
	//! called the resourcehandle is destroyed (on final Close), should call CORE_DELETE on the derived handle type
	typedef void (*DestroyResourceHandleCallback)( ResourceHandleBase* handle );

	//! when acquire is called pumping queues may be required to avoid deadlock. can be called on any thead so beware!
	typedef void (*AcquirePumpCallback)();


	//! register callback for a new resource type
	void registerResourceType (		uint32_t type, //!< unique ID for resource
									CreateResourceCallback pCreate,  //!< required callback 
									DestroyResourceCallback pDestroy,//!< required callback 
									const size_t resourceHandleSize, //!< required total size of the handle, will be zero'd on handle creation
									DestroyResourceHandleCallback pDestroyHandle = NULL,//!< optional callback NULL uses default destroy
									ChangeResourceCallback pChange= 0, //!< Optional callback when change occurs
									const std::string dir = ""			//!< Optional directory prefix
									);
	void registerAcquirePump( AcquirePumpCallback pump );

	// Load or Create a resource, for load name is filename, data is the resource specific load structure, 
	// for create name is internal name to allow caching or not, data is the resource specific create structure
	template< uint32_t type >
	const TypedResourceHandle<type>* loadCreateResource( const char* pName, const void* pData, const size_t sizeofData, uint32_t flags );

	//! used when the resource is finished
	template<uint32_t type>
	void closeResource( const TypedResourceHandle<type>* handle );

	//! flushes a resource fromt he caches
	template<uint32_t type>
	void flushResource( const char* pName, uint32_t flags );

	void baseCloseResource( ResourceHandleBase* pHandle );

	template<uint32_t type>
	TypedResourceHandle<type>* cloneResource( const TypedResourceHandle<type>* handle );

	ResourceHandleBase* baseCloneResource( ResourceHandleBase* pHandle );

	void internalProcessManifest( uint16_t numEntries, struct ManifestEntry* entries );
	void internalCloseManifest( uint16_t numEntries, struct ManifestEntry* entries );

	void internalAcquireComplete( const ResourceHandleBase* _handle, std::shared_ptr<ResourceBase>& _resource );

	void internalPumpAcquirePump();
private:
	//! Internal AcquireResource used by TypedResourceHandle
	template<uint32_t type>
		std::shared_ptr<Resource<type> > internalAcquireResource( const ResourceHandleBase* const pHandle );


	const ResourceHandleBase* implOpenResource(  const char* pName, const void* pData, const size_t sizeofData, uint32_t type, RESOURCE_FLAGS flags );
	void implFlushResource( const char* pName, uint32_t type, RESOURCE_FLAGS flags );

	std::shared_ptr<ResourceBase> implAcquireResource( ResourceHandleBase* pHandle );

	class ResourceManImpl&	impl;
};


template<uint32_t type>
const TypedResourceHandle<type>* ResourceMan::loadCreateResource( const char* pName, const void* pData, const size_t sizeofData, uint32_t flags ) {
	const ResourceHandleBase* pRHB = implOpenResource( pName, pData, sizeofData, type, (RESOURCE_FLAGS)flags );
	const TypedResourceHandle<type>* pRH = (const TypedResourceHandle<type>*) pRHB;
	return pRH;
}


template<uint32_t type>
void ResourceMan::closeResource( const TypedResourceHandle<type>* pHandle ) {
	baseCloseResource( const_cast<ResourceHandleBase*>(static_cast<const ResourceHandleBase*>(pHandle)) );
}

template<uint32_t type>
TypedResourceHandle<type>* ResourceMan::cloneResource( const TypedResourceHandle<type>* pHandle ) {
	return (TypedResourceHandle<type>*) baseCloneResource( const_cast<ResourceHandleBase*>(static_cast<const ResourceHandleBase*>(pHandle)) );
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
			if( acquiredp ) { return acquiredp; }

			ResourceMan::get()->internalPumpAcquirePump();
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
