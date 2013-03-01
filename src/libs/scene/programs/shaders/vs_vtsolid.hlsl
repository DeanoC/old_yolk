#include "shared_structs.hlsl"
#include "constant_blocks.glsl"

struct VS_IN {
	float4 position : VE_POSITION;
	float3 normal : VE_NORMAL;
};
struct VS_OUT {
	float4 position : SV_POSITION;
	float3 viewNormal : NORMAL;
	float3 viewPosition : TEXCOORD0;
};

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	output.position = mul( input.position, matrixWorldViewProjection );
	output.viewNormal = mul( float4(input.normal,0), matrixWorldViewIT ).xyz;
	output.viewPosition = mul( input.position, matrixWorldView ).xyz;
	return output;
}
