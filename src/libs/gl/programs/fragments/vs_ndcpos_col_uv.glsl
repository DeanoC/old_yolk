#include "clforgl.glsl"
#include "constant_blocks.glsl"

layout( location = VE_POSITION ) in vec4 inPosition;
layout( location = VE_COLOUR0 ) in vec4 inColour;
layout( location = VE_TEXCOORD0 ) in vec2 inUV;

layout( location = 0 ) out vec4 gl_Position;
layout( location = 1 ) out smooth vec4 outColour;
layout( location = 2 ) out smooth vec2 outUV;

void main() {
	vec4 ndcSpace = inPosition * matrixViewProjection;
	gl_Position = ndcSpace;
	outColour = inColour;
	outUV = inUV;
}
