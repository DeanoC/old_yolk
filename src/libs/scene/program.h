#pragma once

//!-----------------------------------------------------
//!
//! \file program.h
//! A Program is a resource made of a set of GPU shaders
//! for the different parts of the GPU
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_PROGRAM_H_ )
#define YOLK_SCENE_PROGRAM_H_

#include <core/resources.h>

namespace Scene {
	static const uint32_t ProgramType = RESOURCE_NAME('G','P','R','G');

	enum SHADER_TYPES {
		ST_VERTEX,
		ST_FRAGMENT,
		ST_GEOMETRY,
		ST_HULL,
		ST_DOMAIN,
		ST_COMPUTE,
		MAX_SHADER_TYPES
	};

	class Program : public Core::Resource<ProgramType> {
	public:
		friend class ProgramMan;
		friend class ResourceLoader;
		struct CreationInfo {
			void* data[MAX_SHADER_TYPES]; //platform specific partial compile from loader
		};

		virtual ~Program(){};

		// bit flags of CONSTANT_FREQ tell which buffer are used by this program
		uint32_t getUsedBuffers() const { return usedBuffers; }

		virtual bool usesConstantBuffer( const Scene::SHADER_TYPES type, const char* bufferName ) const = 0; 
		virtual bool usesConstantBuffer( const char* bufferName ) const = 0;	
		// a program will have a specific 'offset' for each variable this gets it
		// not fast use at init time
		virtual uint32_t getVariableOffset( const char* bufferName, const char* name ) const = 0;

	protected:
		static const void* internalPreCreate( const char* pName, const CreationInfo* creation );
		static Program* internalCreate( const void* creation );

		Program(){};
		uint32_t			usedBuffers;
	};

	typedef const Core::ResourceHandle<ProgramType, Program> ProgramHandle;
	typedef ProgramHandle* ProgramHandlePtr;
	typedef std::shared_ptr<Program> ProgramPtr;

}

#endif