#include "shared_structs.hlsl"

struct FS_IN {
	float4 position : SV_POSITION;
	vec4 colour : COLOR0;
};
struct FS_OUT {
	vec4 colour0 : SV_TARGET0;
};

FS_OUT main( FS_IN input ) {
	FS_OUT output;
	output.colour0 = input.colour;
	return output;
}
