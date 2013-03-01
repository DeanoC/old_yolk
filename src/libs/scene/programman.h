#pragma once
#if !defined( YOLK_SCENE_PROGRAMMAN_H_ )
#define YOLK_YOLK_PROGRAMMAN_H_
//!-----------------------------------------------------
//!
//! \file prgramman.h
//! handles the shader system / compiler etc.
//!
//!-----------------------------------------------------

#if !defined( YOLK_SCENE_PROGRAM_H_ )
#	include "program.h"
#endif

#if !defined( YOLK_CORE_SINGLETON_H_ )
#	include "singleton.h"
#endif

namespace Core {
	class ResourceHandleBase;
};

namespace Scene {

	class ProgramMan : public Core::Singleton<  ProgramMan, true > {
	public:
		friend class Program;
		void initDefaultPrograms();

		void registerProgramSource( const char* name, const char* source );
	protected:
		virtual void* compileShader( Scene::SHADER_TYPES type, const std::string& shader ) = 0;
		virtual Scene::Program* linkProgram( const Program::CreationInfo* creation ) = 0;

		virtual void buildConstantTables( Scene::Program* prg ); // override as required default impl uses program interfaces to complete
		virtual std::string getSrcHeaderStatement() const { return std::string(); }; // override as required
		virtual void preprocessPatchSource( std::string& src ); // override if extra patching is required (base removes #includes)

		const char* getProgramSource( const std::string& prgName ) const;

		void* parseProgram( const char* pName, const Program::CreationInfo* );
		Program* completeProgram( const Program::CreationInfo* creation );

		void preprocess( const std::string& srcString, std::string& outsrc );
		bool preprocessIncludes( std::string& src );

		typedef std::unordered_map< std::string, const char*> NameSourceMap;
		NameSourceMap programSrc;

	};
}

#endif