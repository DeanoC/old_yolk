/**
 @file	Z:\Projects\wierd\source\gl\programpipelineobject.cpp

 @brief	Implements the programpipelineobject class.
 */
#include "gl.h"
#include "programpipelineobject.h"

namespace Gl {

ProgramPipelineObject::ProgramPipelineObject() {
	generateName( MNT_PROGRAM_PIPELINE_OBJECT );
}

ProgramPipelineObject::~ProgramPipelineObject() {
	unbind();
}

void ProgramPipelineObject::bind() {
	glBindProgramPipeline( name );
	GL_CHECK
}

void ProgramPipelineObject::unbind() {
	glBindProgramPipeline( 0 );
	GL_CHECK
}

void ProgramPipelineObject::attach( PPO_STAGE stage, const ProgramPtr& prg ) {
	glUseProgramStages( name, stage, prg->getName() );
	GL_CHECK
}

void ProgramPipelineObject::detach( PPO_STAGE stage ) {
//	glUseProgramStages( name, stage, 0 );
}

}
