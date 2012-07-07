#pragma once

//!-----------------------------------------------------
//!
//! \file program.h
//! A Gl Program is a resource representing a compiled
//! GPU shader
//!
//!-----------------------------------------------------
#if !defined(WIERD_GL_PROGRAM_H)
#define WIERD_GL_PROGRAM_H

#include <core/resources.h>
#include "memory.h"

namespace Gl {
	static const uint32_t ProgramRType = RESOURCE_NAME('G','P','R','G');

	enum CONSTANT_FREQ_BLOCKS {
		CF_STATIC = 0,
		CF_PER_FRAME,
		CF_PER_PIPELINE,
		CF_PER_VIEWS,
		CF_PER_TARGETS,
		CF_STD_OBJECT,

		CF_USER_BLOCKS,
		CF_MAX_BLOCKS = 16,
	};

	enum PROGRAM_TYPE {
		PT_VERTEX = GL_VERTEX_SHADER,
		PT_FRAGMENT = GL_FRAGMENT_SHADER,
		PT_GEOMETRY = GL_GEOMETRY_SHADER,
		PT_TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		PT_TESS_EVAL = GL_TESS_EVALUATION_SHADER,

		MAX_PROGRAM_TYPE = 5
	};

	class Program : public Memory,
					public Core::Resource<ProgramRType> {
	public:
		friend class ShaderMan;
		Program();
		struct CreationStruct {
			int				numTransformFeedbackItems;
			bool			interleavedItems;
			const char*		transformFeedbackItemNames[ 16 ]; // max items
		};
		struct LoadStruct {};

		// bit flags of CONSTANT_FREQ tell which buffer are used by this program
		uint32_t getUsedBuffers() const { return usedBuffers; }

	protected:
		uint32_t			usedBuffers;
		bool				wholeProgram;
		// non pipelinr objects have shaders embedded
		Memory::Name		wholeProgramShaders[ MAX_PROGRAM_TYPE ];
	};

	typedef const Core::ResourceHandle<ProgramRType, Program> ProgramHandle;
	typedef ProgramHandle* ProgramHandlePtr;
	typedef std::shared_ptr<Program> ProgramPtr;

}

#endif