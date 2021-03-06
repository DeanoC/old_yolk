struct FS_IN {
	float4 position : SV_POSITION;
	float2 uv 		: TEXCOORD0;
	float4 col 		: COLOR0;
};

struct FS_OUT {
	float4 colour0 : SV_TARGET0;	
};

Texture2D inputTexture : register( t0 );
SamplerState linearSampler : register( s0 );

FS_OUT main( FS_IN input) {
	FS_OUT output;

	const float4 gathered = inputTexture.Gather( linearSampler, input.uv );
	const float dist = gathered.y + gathered.w - gathered.x - gathered.z;

//	const float dist = inputTexture.Sample( linearSampler, input.uv ).r;
	const float smoothing = fwidth( dist ) * 1; // TODO change 1 to a 'sharpness'
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, dist);
	output.colour0 = float4( alpha.xxx * input.col.xyz, 1.0f );//alpha );

	return output;
}
