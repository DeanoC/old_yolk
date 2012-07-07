#include "clforgl.glsl"
#include "constant_blocks.glsl"
#include "scenecapturestruct.glsl"

// input from geometry stages
in uniform vec4 gl_FragCoord;
in uniform bool gl_FrontFacing;
in uniform uint gl_SampleMaskIn[];
layout(location = 0) in flat uint primitiveId;
layout(location = 1) in float3 barycentric;
layout(location = 2) in flat float wflip;

// out put to render targets
layout( location = 0 ) out vec4 outColour0;
layout( location = 1 ) out vec4 outColour1;

// atomic counter + fragment store
layout( binding = 0, offset = 0 ) uniform atomic_uint fragmentCounter;
layout( rgba32ui, binding = 0 ) writeonly uniform uimage1D fragments; // each element == FragmentLink
layout( r32ui, binding = 1 ) uniform uimage1D heads;

void main() {
	const uint fragIndex = atomicCounterIncrement( fragmentCounter );
	const uvec2 pixCoords = uvec2(gl_FragCoord.xy);

	const float3 bary = barycentric;

	float linearDepth = gl_FragCoord.z;//(gl_FragCoord.z / gl_FragCoord.w) / zPlanes.y; //gl_FragCoord.w;//1.0f - gl_FragCoord.w;//
	int iLinearDepth = int(linearDepth * float(1 << 23));

	FragmentLink fragLink;
	fragLink.fragment.depth = ((iLinearDepth << 8) >> 8) & 0x00FFFFFF;										// 24 bit depth 
	fragLink.fragment.depth = bitfieldInsert( fragLink.fragment.depth, gl_SampleMaskIn[0], 24, 8);		// + 8 bit coverage mask
	fragLink.fragment.primitiveId = primitiveId;														// 31 bit primitive ID
	fragLink.fragment.primitiveId = bitfieldInsert( fragLink.fragment.primitiveId, uint(gl_FrontFacing), 31, 1 ); // 1 bit facing
	fragLink.fragment.bary = packHalf2x16( bary.xy );
	fragLink.nextIndex = imageAtomicExchange( heads, int((pixCoords.y * targetDims.x) + pixCoords.x), fragIndex );
	imageStore( fragments, int(fragIndex), uvec4(fragLink.fragment.depth, fragLink.fragment.primitiveId, fragLink.fragment.bary, fragLink.nextIndex) );

	outColour0 = vec4( 1.0f/255.0f, 0, 0, 0);
	outColour1 = vec4( linearDepth, 0, 0, linearDepth + 1.0f/float(1 << 24));
}
