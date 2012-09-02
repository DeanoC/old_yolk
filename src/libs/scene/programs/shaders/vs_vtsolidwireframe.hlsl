#include "shared_structs.hlsl"
#include "constant_blocks.glsl"

struct VS_IN {
	float4 position : VE_POSITION;
};
struct VS_OUT {
	float4 position : SV_POSITION;
	float3 worldPosition : TEXCOORD0;
};

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	output.position = mul( input.position, matrixWorldViewProjection );
	output.worldPosition = mul( input.position, matrixWorld ).xyz;
	return output;
}
