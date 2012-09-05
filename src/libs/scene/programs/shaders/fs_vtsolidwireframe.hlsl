#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
#include "vtstructs.hlsl"

struct FS_IN {
	float4 position		: SV_Position;
	uint coverage 		: SV_Coverage;
	float3 viewNormal 	: TEXCOORD0;
	float3 edgeH		: TEXCOORD1;
};

// normal encoding and decoding function from Andrew Lauritzen Intel Deferred Shading Sample
float2 encodeSphereMap(float3 n) {
    float oneMinusZ = 1.0f - n.z;
    float p = sqrt( (n.x * n.x) + (n.y * n.y) + (oneMinusZ * oneMinusZ) );
    return (n.xy / p) * 0.5f + 0.5f;
}

float evalMinDistanceToEdges(in float3 edgeH) {
    float dist;

	float3 ddxHeights = ddx( edgeH );
	float3 ddyHeights = ddy( edgeH );
	float3 ddHeights2 =  (ddxHeights * ddxHeights) + (ddyHeights * ddyHeights);
	
    float3 pixHeights2 = (edgeH *  edgeH) / ddHeights2 ;
    
    dist = sqrt( min ( min (pixHeights2.x, pixHeights2.y), pixHeights2.z) );
    
    return dist;
}

void main( in FS_IN input, out uint4 og0 : SV_Target0, out float4 og1 : SV_Target1 ) {
    // Compute the shortest distance between the fragment and the edges.
    float dist = evalMinDistanceToEdges( input.edgeH );

    // these are just used for readbility, will be removed by the compiler
    // crap but HLSL can't handle target semantics on structure with decent names
    VtGBuffer0 gb0;
    VtGBuffer1 gb1;

    og0.x = gb0.materialIndex = (uint) materialIndex.x;
    og0.y = gb0.coverage = (uint)(input.coverage);
    og1.xy = gb1.normal = encodeSphereMap( normalize( input.viewNormal ) );
    og1.z = gb1.edgeDist = dist;

    // spare fields
    og0.zw = 0;
    og1.w = 0;
}
