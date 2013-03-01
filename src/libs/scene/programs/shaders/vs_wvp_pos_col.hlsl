#include "shared_structs.hlsl"
#include "constant_blocks.glsl"


struct VS_IN {
	float4 position : VE_POSITION;
	float4 colour 	: VE_COLOUR0;
};
struct VS_OUT {
	float4 position : SV_POSITION;
	float4 colour : COLOR0;
};

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	output.colour = input.colour;
	output.position = mul( input.position, matrixWorldViewProjection );
	return output;
}
