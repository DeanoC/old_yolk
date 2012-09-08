#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
#include "vtstructs.hlsl"

struct FS_IN {
	float4 position		: SV_Position;
	float3 viewNormal 	: TEXCOORD0;
	nointerpolation uint coverage		: SV_Coverage;
};

// normal encoding and decoding function from Andrew Lauritzen Intel Deferred Shading Sample
float2 encodeSphereMap(float3 n) {
    float oneMinusZ = 1.0f - n.z;
    float p = sqrt( (n.x * n.x) + (n.y * n.y) + (oneMinusZ * oneMinusZ) );
    return (n.xy / p) * 0.5f + 0.5f;
}

RWTexture2D<uint> fragmentCounter : register( u1 );
RWStructuredBuffer<VtTransparentFragment> fragments : register( u2 );

[earlydepthstencil]
void main( in FS_IN input ) {
	uint2 esm = (uint2) (encodeSphereMap( input.viewNormal ) * 1023.f);
	uint depth = (uint) (input.position.z * 65535.f);

	uint2 fcDim;
    uint dummy;
    fragmentCounter.GetDimensions( fcDim.x, fcDim.y );
	uint2 stCoord = input.position.xy;

	uint fragCount;
	InterlockedAdd( fragmentCounter[ stCoord ], 1, fragCount );
	if( fragCount >= TOTAL_TRANS_OR_AA_FRAGS ) {
		discard;
	}

	VtTransparentFragment frag;
	frag.matIndex_normal = 
		((materialIndex.x & 0xFFF) << 20) |
		((esm.x & 0x3FF) << 10) | ((esm.y & 0x3FF) << 0);
	frag.cov_depth = (( input.coverage & 0xFF)<< 24) |
					(depth & 0xFFFF);

	uint fragIndex = ((fcDim.x * stCoord.y) + stCoord.x) *  TOTAL_TRANS_OR_AA_FRAGS;
	fragments[ fragIndex + fragCount ] = frag;
}
