#include "clforgl.glsl"
#include "constant_blocks.glsl"

layout( location = VE_POSITION ) in vec4 inPosition;
layout( location = VE_NORMAL ) in vec3 inNormal;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

layout( location = 0 ) out vec3 outWorldPosition;
layout( location = 1 ) out vec3 outNormal;


void main() {
	gl_Position = inPosition * matrixWorldViewProjection;

	outWorldPosition = (inPosition * matrixWorld).xyz;
	outNormal = (float4(inNormal,0) * matrixWorldIT).xyz;
	outNormal = normalize( outNormal );
}
