#pragma once

//!-----------------------------------------------------
//!
//! \file programman.h
//! handles the program/kernel system / compiler etc.
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_PROGRAMMAN_H)
#define WIERD_CL_PROGRAMMAN_H

namespace Core {
	class ResourceHandleBase;
};

#include "programlibrary.h"

namespace Cl {

	class ProgramMan {
	public:
		ProgramMan();

		void registerLibrarySource( const char* name, const char* pSrc );
			
		ProgramLibrary* internalCreate( const Core::ResourceHandleBase* baseHandle, const char* pName, const ProgramLibrary::CreationStruct* creation );
	private:
		const char* getProgramLibrarySource( const std::string& prgName ) const;

		void preprocessIncludes( std::string& src, std::vector<std::string>& fragments );
		void preprocessSharedSource( std::string& src );

		typedef std::unordered_map<std::string, const char*> NameSourceMap;
		NameSourceMap programSrc;

	};
}

#endif