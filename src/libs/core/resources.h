//!-----------------------------------------------------
//!
//! \file resources.h
//!   Copyright (C) 2004 by Dean Calver                                     
//!   deano@rattie.demon.co.uk                                              
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_RESOURCES_H
#define WIERD_CORE_RESOURCES_H


#include "core_utils.h"
#include "resourcebase.h"
#include "resourceman.h"

//! helper macro for resource names
#define RESOURCE_NAME(a,b,c,d) Core::GenerateID<a,b,c,d>::value

namespace Core {

//! passed to resource create call backs
template< uint32_t type, class rclass, typename resultType = void*, uint32_t forcedCreateFlags = 0 >
class AsyncResourceHandle : public Core::ResourceHandle<type> {
public:
	static const uint32_t Type = type;
	typedef rclass								ResourceClass;
	typedef Core::shared_ptr<ResourceClass>	ResourcePtr;

	// helper to acquire an class that inherits off Resource<type>
	ResourcePtr Acquire() const {
	  return Core::static_pointer_cast<ResourceClass>( Core::ResourceHandle<type>::BaseAcquire() );
	}
	static const AsyncResourceHandle<type, rclass, resultType,forcedCreateFlags>* Load( const char* _name, const struct ResourceClass::LoadStruct* _data = NULL, Core::RESOURCE_FLAGS _flags = Core::RMRF_PRELOAD ) {
	  return static_cast<const AsyncResourceHandle<type, rclass, resultType,forcedCreateFlags>*>( Core::ResourceMan::Get()->LoadCreateResource<Type>( _name, _data, sizeof(*_data), _flags | forcedCreateFlags | Core::RMRF_LOADOFFDISK ) );
	}
	static const AsyncResourceHandle<type, rclass, resultType,forcedCreateFlags>* Create( const char* _name, const struct ResourceClass::CreationStruct* _data = NULL, Core::RESOURCE_FLAGS _flags = Core::RMRF_PRELOAD ) {
	  return static_cast<const AsyncResourceHandle<type, rclass, resultType,forcedCreateFlags>*>( Core::ResourceMan::Get()->LoadCreateResource<Type>( _name, _data, sizeof(*_data), _flags | forcedCreateFlags | Core::RMRF_INMEMORYCREATE ) );
	}

	static void Flush( const char* _name, Core::RESOURCE_FLAGS _flags = Core::RMRF_PRELOAD ) {
	  Core::ResourceMan::Get()->FlushResource<Type>( _name, _flags );
	}

	void Close() const {
	  Core::ResourceMan::Get()->CloseResource<Type>( const_cast<AsyncResourceHandle<type, rclass, resultType,forcedCreateFlags>*>(this) );
	}

	static void DestroyHandle( Core::ResourceHandleBase* rhb ) {
	  AsyncResourceHandle<type, rclass, resultType>* handle = static_cast<AsyncResourceHandle<type, rclass, resultType,forcedCreateFlags>*>(rhb);
	  CORE_DELETE handle;
	}
  
	mutable resultType				asyncResultPtr;
#if PLATFORM == WINDOWS
	mutable HRESULT					asyncResult;
#else 
	mutable void*					asyncResult;
#endif
};

template< typename arh >
class ScopedAsyncResourceHandle {
public:
	typedef typename arh::ResourcePtr ResourcePtr;

	explicit ScopedAsyncResourceHandle() : handle(nullptr) {}

	explicit ScopedAsyncResourceHandle( const arh* _handle ) : handle(_handle) {}
	~ScopedAsyncResourceHandle() { if( handle ) handle->Close(); }

	void reset( const arh* _handle ){ handle = _handle; }

	ResourcePtr Acquire() const { return handle->Acquire(); }

	const arh* get() { return handle; }
private:
	ScopedAsyncResourceHandle& operator= (const ScopedAsyncResourceHandle&);

	typedef typename arh::ResourcePtr ResourcePtr;
	const arh*	 handle;	
};

//! most resource destroyers are the same, this automates the construction
template< typename T >
void SimpleResourceDestroyer( Core::shared_ptr<Core::ResourceBase>& spBase ) {
	Core::shared_ptr<T> spActual = Core::static_pointer_cast<T>(spBase);
	spBase.reset(); // the order of these two reset is vital for correct
	spActual.reset(); // destruction
}


}	//namespace Core

#endif //WIERD_CORE_RESOURCES_H
