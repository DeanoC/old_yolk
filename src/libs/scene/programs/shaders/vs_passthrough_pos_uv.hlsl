#include "shared_structs.hlsl"
#include "constant_blocks.glsl"

struct VS_IN {
	float4 position : VE_POSITION;
	float2 uv		: VE_TEXCOORD0;
};
struct VS_OUT {
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
};

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	output.position = input.position;
	output.uv = input.uv;
	return output;
}
