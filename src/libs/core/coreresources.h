//!-----------------------------------------------------
//!
//! \file coreresources.h
//! holds all the core resources stuff etc.
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_CORERESOURCES_H
#define WIERD_CORE_CORERESOURCES_H


#include "resources.h"
#include "resource_dir.h"

namespace Core {


void InstallResourceTypes();

static const uint32_t TextType						=	RESOURCE_NAME('T','E','X','T');

//! An actual text resource
struct TextResource : public Core::Resource<TextType> {
	struct CreationStruct {};
	struct LoadStruct {};

	boost::scoped_array<char>		m_saText;
};

//! text Type
//! A text file resource handle typedef
typedef const Core::AsyncResourceHandle<TextType, TextResource> TextResourceHandle;
typedef TextResourceHandle*										TextResourceHandlePtr;
typedef std::shared_ptr<TextResource>							TextResourcePtr;

//! Manifests
// a binary file that list a bunch of resources, allowing easy grouping of multiple
// resources in one load.
//! A text file resource handle typedef
static const uint32_t ManifestType					=	RESOURCE_NAME('M','A','N','I');
// a single entry
struct ManifestEntry {
	uint32_t							type;
	const char*							name;
	const Core::ResourceHandleBase*		handle;
};

struct ManifestResource : public Core::Resource<ManifestType> {
	struct CreationStruct {};
	struct LoadStruct {};

	uint16_t							numEntries;
	boost::scoped_array<ManifestEntry>	entries;
};

typedef const Core::AsyncResourceHandle<ManifestType, ManifestResource >	ManifestResourceHandle;
typedef ManifestResourceHandle*												ManifestResourceHandlePtr;
typedef std::shared_ptr<ManifestResource>									ManifestResourcePtr;


}

#endif // WIERD_CORE_CORERESOURCES_H
