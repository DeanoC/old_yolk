struct FS_IN {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

struct FS_OUT {
	float4 colour0 : SV_TARGET0;	
};

Texture2DMS<float4,8> inputTexture : register( t0 );

FS_OUT main( FS_IN input) {
	FS_OUT output;

	// todo move getdimension to a lower frequency shader (GS or VS)
	float width;
	float height;
	uint samples;
	inputTexture.GetDimensions( width, height, samples );
	uint2 coord = uint2( (uint)(width * input.uv[0]), (uint)(height * input.uv[1]) );

	output.colour0 = float4(0,0,0,0);
	int i;
	[unroll]
	for( i = 0; i < 8; ++i ) {
		output.colour0 += inputTexture.Load( coord, i );
	}

	output.colour0 /= 8;

	return output;

}
