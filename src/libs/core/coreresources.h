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

#define FOP( x, y ) CORE_ALIGN(8) union { x p; struct { uint32_t h; uint32_t l; } o; } y

namespace Core {


void InstallResourceTypes();

static const uint32_t TextType						=	RESOURCE_NAME('T','E','X','T');

//! An actual text resource
struct TextResource : public Core::Resource<TextType> {
	struct CreationInfo {};

	boost::scoped_array<char>		text;
};

//! text Type
//! A text file resource handle typedef
typedef const Core::ResourceHandle<TextType, TextResource> TextResourceHandle;
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
	FOP( const char*,							name );
	FOP( const Core::ResourceHandleBase*,		handle );
};

struct ManifestResource : public Core::Resource<ManifestType> {
	struct CreationInfo {};

	uint16_t							numEntries;
	boost::scoped_array<ManifestEntry>	entries;
};

typedef const Core::ResourceHandle<ManifestType, ManifestResource >	ManifestResourceHandle;
typedef ManifestResourceHandle*												ManifestResourceHandlePtr;
typedef std::shared_ptr<ManifestResource>									ManifestResourcePtr;


static const uint32_t BinPropertyType					=	RESOURCE_NAME('P','R','O','P');

struct BinProperty {
	enum Flags {
		BPF_SMALL_INLINE = BIT(0), // TODO pack small data inline instead of pointers
		BPF_ANIMATED = BIT(1),
	}; // 5 bits for flags

	enum Type {
		BPT_FLOAT 			= 0,
		BPT_UINT8			= 1,
		BPT_UINT16			= 2,
		BPT_UINT32			= 3,
		BPT_INT32 			= 4,
		BPT_UINT64			= 5,
		BPT_INT64 			= 6,
		BPT_STRING			= 7
	}; // 3 bits == 8 types

	uint8_t		flagsAndType;
	uint8_t		padd[3];
	uint32_t	count; 			// upto 4 billion values, 0 not represented ActualCount = count + 1

	FOP( const char*, 	name );
	FOP( void*,			data );
};

struct BinPropertyResource : public Core::Resource<BinPropertyType> {
	struct CreationInfo {};

	uint16_t getNumProperties() { return numEntries; }
	std::string getName( uint16_t index ) { return std::string( entries[index].name.p ); }	
	BinProperty::Type getType( uint16_t index ) { return (BinProperty::Type)(entries[index].flagsAndType & 0x7); }
	BinProperty::Flags getFlags( uint16_t index ) { return (BinProperty::Flags)((entries[index].flagsAndType & ~0x7)>>3); }
	unsigned int getCount( uint16_t index ) { return entries[index].count+1; }
	template< class type> const type* getAs( uint16_t index ) { return (const type*)(entries[index].data.p); }

	uint32_t							totalSize;
	uint16_t							numEntries;
	boost::scoped_array<BinProperty>	entries;
};

//! Binary Property resource
typedef const Core::ResourceHandle<BinPropertyType, BinPropertyResource> BinPropertyResourceHandle;
typedef BinPropertyResourceHandle*										BinPropertyResourceHandlePtr;
typedef std::shared_ptr<BinPropertyResource>							BinPropertyResourcePtr;



}

#undef FOP

#endif // WIERD_CORE_CORERESOURCES_H
