#include "clforgl.glsl"
#include "constant_blocks.glsl"

layout( location = VE_POSITION ) in vec4 inPosition;
layout( location = VE_COLOUR0 ) in vec4 inColour;
layout( location = 0 ) out vec4 gl_Position;
layout( location = 1 ) out smooth vec4 outColour;

void main() {
	vec4 ndcSpace = inPosition * matrixViewProjection;
	gl_Position = ndcSpace;
	outColour = inColour;
}
