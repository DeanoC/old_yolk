#include "shared_structs.hlsl"
#include "constant_blocks.glsl"


struct VS_IN {
	float4 position : VE_POSITION;
};
struct VS_OUT {
	float4 position : SV_POSITION;
};

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	output.position = mul( input.position, matrixWorldViewProjection );
	return output;
}
