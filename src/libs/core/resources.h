//!-----------------------------------------------------
//!
//! \file resources.h
//!   Copyright (C) 2004 by Dean Calver                                     
//!   deano@rattie.demon.co.uk                                              
//!
//!-----------------------------------------------------

#pragma once

#ifndef YOLK_CORE_RESOURCES_H_
#define YOLK_CORE_RESOURCES_H_

#ifndef YOLK_CORE_UTILS_H_
#	include "core_utils.h"
#endif
#ifndef YOLK_CORE_RESOURCEBASE_H_
#	include "resourcebase.h"
#endif
#ifndef YOLK_CORE_RESOURCEMAN_H_
#	include "resourceman.h"
#endif

//! helper macro for resource names
#define RESOURCE_NAME(a,b,c,d) Core::GenerateID<a,b,c,d>::value

namespace Core {

//! passed to resource create call backs
template< uint32_t _type, class rclass, uint32_t forcedCreateFlags = 0 >
class ResourceHandle : public Core::TypedResourceHandle<_type> {
public:
	static const uint32_t Type = _type;
	typedef rclass														ResourceClass;
	typedef std::shared_ptr<ResourceClass>								ResourcePtr;
	typedef const ResourceHandle<_type, rclass, forcedCreateFlags> 		ThisConstType;

	// helper to acquire a class that inherits off Resource<type>
	ResourcePtr acquire() const {
	  return std::static_pointer_cast<ResourceClass>( ResourceHandleBase::baseAcquire<_type>() );
	}

	// helper to try and acquire a class that inherits off Resource<type>
	ResourcePtr tryAcquire() const {
	  return std::static_pointer_cast<ResourceClass>( ResourceHandleBase::baseTryAcquire<_type>() );
	}

	static ThisConstType* load( const char* _name, const struct ResourceClass::CreationInfo* _data = NULL, Core::RESOURCE_FLAGS _flags = Core::RMRF_NONE) {
	  return static_cast<ThisConstType*>( Core::ResourceMan::get()->loadCreateResource<Type>( _name, _data, sizeof(*_data), _flags | forcedCreateFlags | Core::RMRF_LOADOFFDISK ) );
	}

	static ThisConstType* create( const char* _name, const struct ResourceClass::CreationInfo* _data = NULL, Core::RESOURCE_FLAGS _flags = Core::RMRF_NONE ) {
	  return static_cast<ThisConstType*>( Core::ResourceMan::get()->loadCreateResource<Type>( _name, _data, sizeof(*_data), _flags | forcedCreateFlags | Core::RMRF_INMEMORYCREATE ) );
	}

	static void flush( const char* _name, Core::RESOURCE_FLAGS _flags = Core::RMRF_NONE ) {
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
	void reset( const arh* _handle = nullptr ) { 
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
	CORE_ASSERT( spActual.unique() ); // if this fires something has gone wrong with ref-counting :(:(
	spActual.reset(); // destruction
}


}	//namespace Core

#endif //WIERD_CORE_RESOURCES_H
