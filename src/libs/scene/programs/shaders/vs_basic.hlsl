#include "shared_structs.hlsl"
#include "constant_blocks.glsl"


struct VS_IN {
	float4 position : VE_POSITION;
	float3 normal : VE_NORMAL;
};
struct VS_OUT {
	float4 position : SV_POSITION;
	float4 colour : COLOR0;
};

VS_OUT main( VS_IN input ) {
	VS_OUT output;
	static const vec3 fakeDir = float3( 0.707, 0.707, 0.0 );
	float c = abs( dot( input.normal, fakeDir ) ) + 0.1f;
	output.colour = float4( c, c, c, 1);
	output.position = mul( input.position, matrixWorldViewProjection );
	return output;
}
