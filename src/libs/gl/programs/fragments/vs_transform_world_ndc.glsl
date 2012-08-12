#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
layout( location = VE_POSITION ) in vec4 inPosition;
layout( location = 0 ) out vec4 gl_Position;
layout( location = 1 ) out vec3 outWorldPosition;


void main() {
	gl_Position = inPosition * matrixWorldViewProjection;
	outWorldPosition = vec3( inPosition * matrixWorld );
}
