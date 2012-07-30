#include "clforgl.glsl"
#include "constant_blocks.glsl"
layout( location = VE_POSITION ) in vec4 inPosition;
layout( location = 0 ) out vec4 gl_Position;


void main() {
	gl_Position = inPosition * matrixWorldViewProjection;
}
