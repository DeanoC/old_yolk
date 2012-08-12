#pragma once
/**
 @file	Z:\Projects\wierd\source\gl\gpu_constants.h

 @brief	This allows sharing with glsl or CL uniform blocks
 */
#if !defined( YOLK_GL_GPU_CONSTANTS_H_ )
#define YOLK_GL_GPU_CONSTANTS_H_

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


#define YOLK_GL_GET_OFFSET_IN_BLOCK( b, v ) offsetof( Gl::GPUConstants:: b, v )
#define YOLK_GL_GET_SIZEOF_PRG_VAR( b, v ) sizeof( ((Gl::GPUConstants:: b *)0)->v )
#endif