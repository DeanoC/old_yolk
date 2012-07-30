#include "clforgl.glsl"
#include "constant_blocks.glsl"
layout( location = VE_POSITION ) in vec4 inPosition;
layout( location = VE_NORMAL ) in vec3 inNormal;
layout( location = 0 ) out vec4 gl_Position;
layout( location = 1 ) flat out vec4 outColour;

const vec3 fakeDir = vec3( 0.707, 0.707, 0 );

void main() {
	outColour = vec4( dot( inNormal, fakeDir ).xxx, 1);
	gl_Position = inPosition * matrixWorldViewProjection;
}
