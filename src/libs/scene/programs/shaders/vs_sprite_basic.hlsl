#include "shared_structs.hlsl"
#include "constant_blocks.glsl"


struct VS_IN {
	float4 position : VE_POSITION;
	float2 uv		: VE_TEXCOORD0
	float4 colour 	: VE_COLOUR0;
};
struct VS_OUT {
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0
	float4 colour 	: COLOR0;
};

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	output.uv = input.uv;
	output.colour = input.colour;
	output.position = mul( input.position, matrixWorldViewProjection );
	return output;
}
