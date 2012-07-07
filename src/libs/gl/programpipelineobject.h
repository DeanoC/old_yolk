/**
 @file	Z:\Projects\wierd\source\gl\programpipelineobject.h

 @brief	Declares the programpipelineobject class.
 */
#pragma once
#if !defined( WIERD_GL_PROGRAMPIPELINEOBJECT_H__ )
#define WIERD_GL_PROGRAMPIPELINEOBJECT_H__ 

#include "memory.h"
#include "program.h"

namespace Gl {
	enum PPO_STAGE {
		PPO_VERTEX_STAGE = GL_VERTEX_SHADER_BIT,
		PPO_FRAGMENT_STAGE = GL_FRAGMENT_SHADER_BIT,
		PPO_GEOMETRY_STAGE = GL_GEOMETRY_SHADER_BIT,
		PPO_TESS_CONTROL_STAGE = GL_TESS_CONTROL_SHADER_BIT,
		PPO_TESS_EVAL_STAGE = GL_TESS_EVALUATION_SHADER_BIT,

		PPO_MAX_STAGES = 5,
	};
	class ProgramPipelineObject : public Memory { 
	public:
		ProgramPipelineObject();
		virtual ~ProgramPipelineObject();

		void bind();
		void unbind();

		void attach( PPO_STAGE stage, const ProgramPtr& prg );
		void detach( PPO_STAGE stage );

	};
}

#endif