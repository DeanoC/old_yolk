#pragma once

//!-----------------------------------------------------
//!
//! \file program.h
//! A Gl Program is a resource representing a compiled
//! GPU shader
//!
//!-----------------------------------------------------
#if !defined( YOLK_GL_PROGRAM_H_ )
#define YOLK_GL_PROGRAM_H_

#if !defined( YOLK_GL_MEMORY_H_ )
#	include "memory.h"
#endif

#if !defined( YOLK_SCENE_PROGRAM_H_ )
#	include "scene/program.h"
#endif

namespace Gl {
	
	enum PROGRAM_TYPE {
		PT_VERTEX = GL_VERTEX_SHADER,
		PT_FRAGMENT = GL_FRAGMENT_SHADER,
		PT_GEOMETRY = GL_GEOMETRY_SHADER,
		PT_TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		PT_TESS_EVAL = GL_TESS_EVALUATION_SHADER,

		MAX_PROGRAM_TYPE = 5
	};

	class Program : public Memory, public Scene::Program {
	public:
	protected:
		friend class ShaderMan;
		Program(){};

		bool				wholeProgram;
		// non pipelinr objects have shaders embedded
		Memory::Name		wholeProgramShaders[ MAX_PROGRAM_TYPE ];
	};

}

#endif