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

RWTexture2D<uint> fragmentCounter : register( u1 );
RWStructuredBuffer<VtTransparentFragment> fragments : register( u2 );
const static int MAX_TRANS_FRAGS = 8;
[earlydepthstencil]
void main( in FS_IN input ) {
	uint2 esm = (uint2) (encodeSphereMap( input.viewNormal ) * 255.f);
	uint depth = (uint) (input.position.z * 65535.f);

	uint2 fcDim;
    uint dummy;
    fragmentCounter.GetDimensions( fcDim.x, fcDim.y );
	uint2 stCoord = input.position.xy;
	
	int fragCount;
	InterlockedAdd( fragmentCounter[ stCoord ], 1, fragCount );
	if( fragCount >= 8 ) {
		discard;
	}

	VtTransparentFragment frag;
	frag.matIndex_depth = 
		((materialIndex.x & 0xFFFF) << 16) | (depth & 0xFFFF);
	frag.normal = ((esm.x & 0xFF) << 24) | ((esm.y & 0xFF) << 16);

	// TODO MSAA if required
	uint fragIndex = ((fcDim.x * stCoord.y) + stCoord.x) *  MAX_TRANS_FRAGS;
	fragments[ fragIndex + fragCount ] = frag;
}
