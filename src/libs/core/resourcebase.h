//!-----------------------------------------------------
//!
//! \file resourcebase.h
//!   Copyright (C) 2010 by Dean Calver                                     
//!   deano@rattie.demon.co.uk                                              
//!
//!-----------------------------------------------------
#ifndef WIERD_CORE_RESOURCEHANDLE_H
#define WIERD_CORE_RESOURCEHANDLE_H

#pragma once

#include <boost/weak_ptr.hpp>

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
	uint32_t GetType() const {
		return m_Type;
	}

	//! should only be done by the resource manager
	~ResourceHandleBase() {};

protected:
	ResourceHandleBase( uint32_t type_ ) : m_Type( type_ ) {};

	//! a weak ptr to the actual resource
	mutable Core::weak_ptr<ResourceBase>	m_wpResourceBase;
	uint32_t m_Type;	//!< type this resource handle points to
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
	// helper to acquire an class that inherits off Resource<type>
	template<class T>
	Core::shared_ptr<T> Acquire() const {
		return boost::shared_static_cast<T>( BaseAcquire() );
	}

	//! acquire a typed resource 
	Core::shared_ptr<Resource<type> > BaseAcquire() const;

protected:
	ResourceHandle() {};
	~ResourceHandle() {};
};

} // end Core namespace

#endif // end include guard
