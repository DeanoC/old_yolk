#include "shared_structs.hlsl"
#include "constant_blocks.glsl"


struct VS_IN {
	float2 position		: VE_POSITION;
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
	output.position = mul( float4(input.position,0,1), matrixWorldViewProjection );

	if( colTexMat0.w >= 0.0f ) {
		output.colUV = colTexMat0;
	} else {
		matrix texMat;
		texMat[0] = float4(colTexMat0.xyz,0);
		texMat[1] = texMat1.xyzw;
		texMat[2] = texMat2.xyzw;
		texMat[3] = texMat3.xyzw;
		output.colUV = float4( mul( float4(input.position,0,1), texMat ).xy, 0, -1 ); // -1 Alpha indicates to fragment shader to do a texture lookup
	}

	return output;
}
