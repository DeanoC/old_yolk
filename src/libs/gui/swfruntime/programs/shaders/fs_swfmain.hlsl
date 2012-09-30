#include "shared_structs.hlsl"
#include "constant_blocks.glsl"

struct VS_IN {
	float4 position : SV_POSITION;
	float4 colUV	: TEXCOORD0;
};

Texture2D inputTexture : register( t0 );
SamplerState linearSampler : register( s0 );

struct FS_OUT {
	float4 colour0 : SV_TARGET0;	
};

FS_OUT main( VS_IN input ) {
	FS_OUT output;

	// is colUV a colour or a UV set to lookup a < 0.0f is the decider
	if( input.colUV.a < 0.0f ) {
		 output.colour0 = inputTexture.Sample( linearSampler, input.colUV.xy );
	} else {
		output.colour0 = input.colUV;
	}
	// colour matrix transform
	output.colour0 = (output.colour0 * matrixUser0[0]) + matrixUser0[1];

	return output;
}
