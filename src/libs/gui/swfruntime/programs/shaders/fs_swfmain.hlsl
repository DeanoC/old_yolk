#include "shared_structs.hlsl"
#include "constant_blocks.glsl"

struct VS_IN {
	float4 position : SV_POSITION;
	float4 colUV	: TEXCOORD0;
};


struct FS_OUT {
	float4 colour0 : SV_TARGET0;	
};

FS_OUT main( VS_IN input ) {
	FS_OUT output;

	output.colour0 = float4(input.colUV.xyz,1);

	return output;
}
