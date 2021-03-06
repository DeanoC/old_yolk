#pragma once
/**
 @file	Z:\Projects\wierd\source\gl\gpu_constants.h

 @brief	This allows sharing with glsl or CL uniform blocks
 */
#if !defined( YOLK_SCENE_GPU_CONSTANTS_H_ )
#define YOLK_SCENE_GPU_CONSTANTS_H_

namespace Scene {
	namespace GPUConstants {

// these 3 should only ever be included (no include headers for portability)
#include "programs/shared_structs.hlsl"
#include "programs/constant_blocks.hlsl"
#include "programs/vtstructs.hlsl"

#undef CF_STATIC
#undef CF_PER_FRAME
#undef CF_PER_PIPELINE
#undef CF_PER_VIEWS
#undef CF_PER_TARGETS
#undef CF_STD_OBJECT
#undef CF_PER_MATERIAL
#undef CF_USER_BLOCK_0
#undef CF_USER_BLOCK_1
#undef CF_USER_BLOCK_2
#undef CF_USER_BLOCK_3
#undef CF_USER_BLOCK_4
#undef CF_USER_BLOCK_5
#undef CF_USER_BLOCK_6
#undef CF_USER_BLOCK_7
#undef CF_USER_BLOCK_8

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
	}
	
}


#define YOLK_GPU_GET_OFFSET_IN_BLOCK( b, v ) offsetof( Scene::GPUConstants:: b, v )
#define YOLK_GPU_GET_SIZEOF_PRG_VAR( b, v ) sizeof( ((Scene::GPUConstants:: b *)0)->v )
#endif