struct FS_IN {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

struct FS_OUT {
	float4 colour0 : SV_TARGET0;	
};

Texture2D inputTexture : register( t0 );
SamplerState pointSampler : register( s0 );

FS_OUT main( FS_IN input) {
	FS_OUT output;
	output.colour0 = inputTexture.Sample( pointSampler, input.uv );

	return output;

}
