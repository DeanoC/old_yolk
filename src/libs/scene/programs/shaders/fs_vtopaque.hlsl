#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
#include "vtstructs.hlsl"

struct FS_IN {
	float4 position		: SV_Position;
	float3 viewNormal 	: TEXCOORD0;
};

// normal encoding and decoding function from Andrew Lauritzen Intel Deferred Shading Sample
float2 encodeSphereMap(float3 n) {
    float oneMinusZ = 1.0f - n.z;
    float p = sqrt( (n.x * n.x) + (n.y * n.y) + (oneMinusZ * oneMinusZ) );
    return (n.xy / p) * 0.5f + 0.5f;
}


void main( in FS_IN input, out uint og0 : SV_Target0 ) {
	uint2 esm = (uint2) (encodeSphereMap( input.viewNormal ) * 255.f);
	og0 =	((materialIndex.x & 0xFFFF) << 16)	| 
			((esm.x & 0xFF) << 8) | ((esm.y & 0xFF) << 0);
}
