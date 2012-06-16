//!-----------------------------------------------------
//!
//! \file resourcebase.h
//!   Copyright (C) 2012 by Dean Calver                                     
//!   deano@rattie.demon.co.uk                                              
//!
//!-----------------------------------------------------
#ifndef WIERD_CORE_RESOURCEHANDLE_H
#define WIERD_CORE_RESOURCEHANDLE_H

#pragma once

namespace Core {

enum RESOURCE_FLAGS
{
	RMRF_NONE				= 0,		//! Nothing
	RMRF_LOADOFFDISK		= BIT(1),	//! this is a off disk resource
	RMRF_INMEMORYCREATE		= BIT(2),	//! this is an in memory resource
	RMRF_DONTCACHE			= BIT(3),	//! never re-use/cache this resource
	RMRF_PRELOAD			= BIT(4),	//! start loading the resource before acquire is called in the background

	RMRF_FORCE32BIT			= 0xFFFFFFFF	//! make sure resource flags is always 32 bit (its relied upon in the naming system)
};

//! all resource derive off this
class ResourceBase {};

//! a typesafe resource, each resource should derive off this
//! with there type as the template parameter
template< uint32_t type >
class Resource : public ResourceBase {
public:
protected:
	Resource() {};
	~Resource() {};
};

//! all resource handle derive of this 
class ResourceHandleBase {
public:
	friend class ResourceMan;	

	//! returns the type of this resource
	uint32_t getType() const { return type; }

	//! should only be done by the resource manager
	~ResourceHandleBase() {};


protected:
	//! acquire a typed resource 
	template< uint32_t type_ >
		std::shared_ptr<Resource<type_> > baseAcquire() const;
	//! try and acquire a typed resource 
	template< uint32_t type_ >
		std::shared_ptr<Resource<type_> > baseTryAcquire() const;

	ResourceHandleBase( uint32_t type_ ) : type( type_ ) {};

	mutable std::weak_ptr<ResourceBase>	resourceBase;
	mutable std::atomic<bool> acquiring; // as acquiring can be async, this simply flags that onces been kicked off
	uint32_t type;	//!< type this resource handle points to
};

//! gets returned, every time you want to use the resource
//! call Acquire, only hold onto the shared_ptr returned
//! for short period of times, this allows the resource manager
//! to reload items behind you back. If you only acquire (and
//! don't point to things inside an acquired resource) when you
//! actually use things, live loading will occur automatically
template< uint32_t type >
class ResourceHandle : public ResourceHandleBase {
public:
	// helper to acquire a class that inherits off Resource<type>
	template<class T>
	std::shared_ptr<T> acquire() const {
		return std::static_pointer_cast<T>( baseAcquire() );
	}

	// helper to try and acquire a class that inherits off Resource<type>
	template< class T >
	std::shared_ptr<T> tryAcquire() const {
		return std::static_pointer_cast<T>( baseTryAcquire() );		
	}

protected:
	ResourceHandle() {};
	~ResourceHandle() {};
};

} // end Core namespace

#endif // end include guard
