/**
 @file	Z:\Projects\wierd\source\gl\memory.cpp

 @brief	Implements the gl base memory class.
 All resources are derived from this, given a uniform handling of gpu accessible RAM
 TODO: open cl access
 */
#include "gl.h"
#include "gfx.h"
#include "memory.h"

namespace Gl {

void Memory::generateNames( MEM_NAME_TYPE type, unsigned int num, Name* outNames ) {
	CORE_ASSERT( outNames );
	memset( outNames, 0, num * sizeof(Name) );

	switch( type ) {
		case MNT_VERTEX_SHADER_OBJECT:
		case MNT_FRAGMENT_SHADER_OBJECT:
		case MNT_GEOMETRY_SHADER_OBJECT:
		case MNT_TESS_CONTROL_OBJECT:
		case MNT_TESS_EVAL_SHADER_OBJECT:
			CORE_ASSERT( false && "Shader objects here are actually programs for the pipelines");
			break;
		case MNT_PROGRAM_PIPELINE_OBJECT: glGenProgramPipelines( num, outNames ); break;
		case MNT_PROGRAM_OBJECT:
			for( unsigned int i = 0; i < num; ++i ) {
				outNames[i] = glCreateProgram();
			}
			break;
		default:
/*			if( glGenNamesAMD ) {
				glGenNamesAMD( type, num, outNames );
			} else */{
				switch( type ) {
				case MNT_FRAME_BUFFER: glGenFramebuffers( num, outNames ); break;
				case MNT_RENDER_BUFFER: glGenRenderbuffers( num, outNames ); break;
				case MNT_TRANSFORM_FEEDBACK_BUFFER: glGenTransformFeedbacks( num, outNames ); break;
				case MNT_DATA_BUFFER: glGenBuffers( num, outNames ); break;
				case MNT_PERFORMANCE_MONITOR_OBJECT:  glGenPerfMonitorsAMD( num, outNames ); break;
				case MNT_TEXTURE_OBJECT: glGenTextures( num, outNames ); break;
				case MNT_QUERY_OBJECT: glGenQueries( num, outNames ); break;
				case MNT_VERTEX_ARRAY_OBJECT: glGenVertexArrays( num, outNames ); break;
				case MNT_SAMPLER_OBJECT: glGenSamplers( num, outNames ); break;
				}
			}
			break;
	}
	IF_DEBUG(
		for( unsigned int i = 0; i < num; ++i ) {
			CORE_ASSERT( outNames[i] != 0 );
		}
	);
}
void Memory::deleteNames( MEM_NAME_TYPE type, unsigned int num, Name* inNames ) {
	CORE_ASSERT( inNames );

	switch( type ) {
		case MNT_VERTEX_SHADER_OBJECT:
		case MNT_FRAGMENT_SHADER_OBJECT:
		case MNT_GEOMETRY_SHADER_OBJECT:
		case MNT_TESS_CONTROL_OBJECT:
		case MNT_TESS_EVAL_SHADER_OBJECT:
			for( unsigned int i = 0; i < num; ++i ) {
				glDeleteProgram( inNames[i] );
			}
			break;
		case MNT_PROGRAM_PIPELINE_OBJECT: glDeleteProgramPipelines( num, inNames ); break;
		case MNT_PROGRAM_OBJECT:
			for( unsigned int i = 0; i < num; ++i ) {
				glDeleteProgram( inNames[i] );
			}
			break;

		default:
/*			if( glGenNamesAMD ) {
				glDeleteNamesAMD( type, num, inNames );
			} else */{
				switch( type ) {
				case MNT_FRAME_BUFFER: glDeleteFramebuffers( num, inNames ); break;
				case MNT_RENDER_BUFFER: glDeleteRenderbuffers( num, inNames ); break;
				case MNT_TRANSFORM_FEEDBACK_BUFFER: glDeleteTransformFeedbacks( num, inNames ); break;
				case MNT_DATA_BUFFER: glDeleteBuffers( num, inNames ); break;
				case MNT_PERFORMANCE_MONITOR_OBJECT: glDeletePerfMonitorsAMD( num, inNames ); break;
				case MNT_TEXTURE_OBJECT: glDeleteTextures( num, inNames ); break;
				case MNT_QUERY_OBJECT: glDeleteQueries( num, inNames ); break;
				case MNT_VERTEX_ARRAY_OBJECT: glDeleteVertexArrays( num, inNames ); break;
				case MNT_SAMPLER_OBJECT: glDeleteSamplers( num, inNames ); break;
				}
			}
			break;
	}
}

}