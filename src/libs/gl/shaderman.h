#pragma once

//!-----------------------------------------------------
//!
//! \file shaderman.h
//! handles the shader system / compiler etc.
//!
//!-----------------------------------------------------
#if !defined( YOLK_GL_SHADERMAN_H)
#define YOLK_GL_SHADERMAN_H

#if !defined( YOLK_GL_PROGRAM_H)
#	include "program.h"
#endif

namespace Core {
	class ResourceHandleBase;
};

namespace Gl {

	class ShaderMan {
	public:
		ShaderMan();

		void registerProgramSource( const char* name, const char* source );

		Program* internalCreate( const Core::ResourceHandleBase* baseHandle, const char* pName, const Program::CreationStruct* creation );

		void initDefaultPrograms();
	private:
		const char* getProgramSource( const std::string& prgName ) const;
		Program* internalCreateWholeProgram( const Core::ResourceHandleBase* baseHandle, const char* pName, const Program::CreationStruct* creation );

		void preprocess( std::string& src, int& count, std::string* glsrc );
		void preprocessIncludes( std::string& src, std::list<std::string>& fragments );
		virtual void preprocessPatchSource( std::string& src ) override;
		void buildUniformTables( Program* prg );

		typedef std::unordered_map< std::string, const char*> NameSourceMap;
		NameSourceMap programSrc;

	};
}

#endif