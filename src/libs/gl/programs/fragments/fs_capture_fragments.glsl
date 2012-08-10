#include "clforgl.glsl"
#include "constant_blocks.glsl"

layout( binding = 0 ) uniform sampler2D fragHeader;
layout( r32i, binding = 1 ) uniform restrict iimageBuffer countTemp;
layout( rgba32ui, binding = 2 ) uniform restrict writeonly uimageBuffer fragBuffer;

void main() {
	const ivec2 pixCoords = ivec2(gl_FragCoord.xy);
	const int tempIndex = int( fma( pixCoords.y, targetDims.x, pixCoords.x ) );

	// TODO replce with atomic counter
	int fragIndex = imageAtomicAdd( countTemp, tempIndex, 1 );
	fragIndex = int( texelFetch( fragHeader, pixCoords, 0 ).x );

	const uvec4 result = uvec4( gl_PrimitiveID, 
								floatBitsToInt(gl_FragCoord.z),
								0,
								0 );

	imageStore( fragBuffer, fragIndex, result );

}
