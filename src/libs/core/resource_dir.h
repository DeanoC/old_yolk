//!-----------------------------------------------------
//!
//! \file resource_dir.h
//!   Copyright (C) 2006 by Dean Calver                                     
//!   deano@rattie.demon.co.uk                                              
//!
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_RESOURCE_DIR_H
#define WIERD_CORE_RESOURCE_DIR_H


#include "core_utils.h"
#include "resources.h"


namespace Core
{


struct ResourceManifestEntry {
	ResourceManifestEntry( uint32_t _type, const std::string& _filename ) : 
		type(_type), filename(_filename ) {};
	uint32_t	type;
	std::string filename;
};

typedef std::vector<ResourceManifestEntry> ResourceManifestEntryVector;


//! An entry in the resource directory...
class ResourceDirEntry
{
public:
	// always owned by ResourceDir
	friend class ResourceDir;

	//! the meat returns the typed resource handle
	template<uint32_t type>	
	const TypedResourceHandle<type>* get() const
	{
		CORE_ASSERT( type == m_iType && type == m_pResourceHandle->getType() )
		return (const TypedResourceHandle<type>*) m_pResourceHandle;
	}

	//! return the original filename, mainly a debug aid as almost never needed
	const char* getFilename() const {
		return m_pFilename;
	}
private:
	ResourceDirEntry();
	~ResourceDirEntry();
	uint32_t					m_iType;
	const char*					m_pFilename;
	const ResourceHandleBase*	m_pResourceHandle;
};


class ResourceDir
{
	uint32_t m_iNumEntries;

	ResourceDirEntry**			m_pEntries;
};


}	//namespace Core


#endif
