#include "shared_structs.hlsl"
#include "constant_blocks.glsl"


struct VS_IN {
	float4 position		: VE_POSITION;
};
struct VS_OUT {
	float4 position : SV_POSITION;
	float4 colUV	: TEXCOORD0;
};

constant_buffer( FillData, CF_USER_BLOCK_0 ) {
	float4 colTexMat0	: VE_TEXCOORD0;
	float4 texMat1		: VE_TEXCOORD1;
	float4 texMat2		: VE_TEXCOORD2;
	float4 texMat3		: VE_TEXCOORD3;
}

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	output.position = mul( input.position, matrixWorldViewProjection );
	output.colUV = colTexMat0;
	return output;
}
