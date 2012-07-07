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
template< uint32_t type, class rclass, uint32_t forcedCreateFlags = 0 >
class ResourceHandle : public Core::TypedResourceHandle<type> {
public:
	static const uint32_t Type = type;
	typedef rclass														ResourceClass;
	typedef std::shared_ptr<ResourceClass>								ResourcePtr;
	typedef const ResourceHandle<type, rclass, forcedCreateFlags> 	ThisConstType;

	// helper to acquire a class that inherits off Resource<type>
	ResourcePtr acquire() const {
	  return std::static_pointer_cast<ResourceClass>( ResourceHandleBase::baseAcquire<type>() );
	}

	// helper to try and acquire a class that inherits off Resource<type>
	ResourcePtr tryAcquire() const {
	  return std::static_pointer_cast<ResourceClass>( ResourceHandleBase::baseTryAcquire<type>() );
	}

	static ThisConstType* load( const char* _name, const struct ResourceClass::LoadStruct* _data = NULL, Core::RESOURCE_FLAGS _flags = Core::RMRF_PRELOAD ) {
	  return static_cast<ThisConstType*>( Core::ResourceMan::get()->loadCreateResource<Type>( _name, _data, sizeof(*_data), _flags | forcedCreateFlags | Core::RMRF_LOADOFFDISK ) );
	}

	static ThisConstType* create( const char* _name, const struct ResourceClass::CreationStruct* _data = NULL, Core::RESOURCE_FLAGS _flags = Core::RMRF_PRELOAD ) {
	  return static_cast<ThisConstType*>( Core::ResourceMan::get()->loadCreateResource<Type>( _name, _data, sizeof(*_data), _flags | forcedCreateFlags | Core::RMRF_INMEMORYCREATE ) );
	}

	static void flush( const char* _name, Core::RESOURCE_FLAGS _flags = Core::RMRF_PRELOAD ) {
	  Core::ResourceMan::get()->flushResource<Type>( _name, _flags );
	}

	void close() const {
	  Core::ResourceMan::get()->closeResource<Type>( const_cast<ThisConstType*>(this) );
	}

	ThisConstType* clone() const {
	  return static_cast<ThisConstType*>( Core::ResourceMan::get()->cloneResource<Type>( const_cast<ThisConstType*>(this) ) );
	}
};

template< typename arh >
class ScopedResourceHandle {
public:
	typedef typename arh::ResourcePtr ResourcePtr;

	explicit ScopedResourceHandle() : handle(NULL) {}

	// pass ownership to this, caller should not close this instance!
	explicit ScopedResourceHandle( const arh* _handle ) : handle(_handle) {}

	~ScopedResourceHandle() { if( handle ) handle->close(); }

	// pass ownership to this, any previous handle will be closed
	void reset( const arh* _handle ) { 
		if( handle ) handle->close();
		handle = _handle; 
	}

	ResourcePtr acquire() const { return handle->acquire(); }

	ResourcePtr tryAcquire() const { return handle->tryAcquire(); }

	ScopedResourceHandle& operator= ( const ScopedResourceHandle& rhs ) {
		if( rhs.handle ) {
			this->reset( rhs.handle->clone() );
		} else {
			this->reset( NULL );
		}
		return *this;
	}

	const arh* get() const { return handle; }
private:

	const arh*	 handle;	
};

//! most resource destroyers are the same, this automates the construction
template< typename T >
void SimpleResourceDestroyer( std::shared_ptr<Core::ResourceBase>& spBase ) {
	std::shared_ptr<T> spActual = std::static_pointer_cast<T>(spBase);
	spBase.reset(); // the order of these two reset is vital for correct
	spActual.reset(); // destruction
}


}	//namespace Core

#endif //WIERD_CORE_RESOURCES_H
