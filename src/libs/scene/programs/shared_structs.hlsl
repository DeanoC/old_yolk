#if defined( __OPENCL_VERSION__ )

// opencl

#define layout(x)

#elif GL_core_profile == 1
layout( row_major ) uniform;

// glsl
#define binding(x) layout( std140, binding = x )
#define struct uniform 

#define int2	ivec2
#define int3	ivec3
#define int4	ivec4

#define uint2	uvec2
#define uint3	uvec3
#define uint4	uvec4

#define float2	vec2
#define float3	vec3
#define float4	vec4

#define float16 mat4x4

#else

// C++ pc code here
#define binding(x)

typedef uint32_t			uvec2[2];
typedef uint32_t			uvec3[3];
typedef uint32_t			uvec4[4];
typedef int32_t				ivec2[2];
typedef int32_t				ivec3[3];
typedef int32_t				ivec4[4];
typedef Math::Vector2		vec2;
typedef Math::Vector3		vec3;
typedef Math::Vector4		vec4;
typedef Math::Matrix4x4		mat4x4;

typedef ivec2				int2;
typedef ivec3				int3;
typedef ivec4				int4;
typedef uint32_t			uint;
typedef uvec2				uint2;
typedef uvec3				uint3;
typedef uvec4				uint4;
typedef vec2				float2;
typedef vec3				float3;
typedef vec4				float4;
typedef mat4x4				float16;

#endif