#pragma once
/**
 @file	Z:\Projects\wierd\source\gl\gpu_constants.h

 @brief	This allows sharing with glsl or CL uniform blocks
 */
#if !defined( WIERD_GL_GPU_CONSTANTS_H__ )
#define WIERD_GL_GPU_CONSTANTS_H__

namespace Gl {
	namespace GPUConstants {

#include "programs/clforgl.glsl"
#include "programs/constant_blocks.glsl"
#include "programs/scenecapturestruct.glsl"

#undef CF_STATIC
#undef CF_PER_FRAME
#undef CF_PER_PIPELINE
#undef CF_PER_VIEWS
#undef CF_PER_TARGETS
#undef CF_STD_OBJECT

#undef VE_POSITION		
#undef VE_NORMAL		
#undef VE_BINORMAL		
#undef VE_TANGENT		
#undef VE_TEXCOORD0	
#undef VE_TEXCOORD1	
#undef VE_TEXCOORD2	
#undef VE_TEXCOORD3	
#undef VE_COLOUR0		
#undef VE_COLOUR1		
#undef VE_BONEINDICES
#undef VE_BONEWEIGHTS

#undef layout
#undef uniform
	}
	
}

#define WIERD_GL_VALIDATE_PRG_OFFSETS( p, b, v ) {									\
	GLuint ind = 0;																\
	GLint offset = -1;															\
	const char* name = #v;														\
	glGetUniformIndices( prg->name, 1, &name, &ind );							\
	glGetActiveUniformsiv( prg->name, 1, &ind, GL_UNIFORM_OFFSET, &offset );	\
	CORE_ASSERT( offsetof( Gl::GPUConstants:: b, v ) == offset );						\
	}
#define WIERD_GL_GET_OFFSET_IN_BLOCK( b, v ) offsetof( Gl::GPUConstants:: b, v )
#define WIERD_GL_GET_SIZEOF_PRG_VAR( b, v ) sizeof( ((Gl::GPUConstants:: b *)0)->v )
#endif