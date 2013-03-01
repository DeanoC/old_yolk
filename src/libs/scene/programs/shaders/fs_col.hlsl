struct FS_IN {
	float4 position : SV_POSITION;
	float4 col 		: COLOR0;
};

struct FS_OUT {
	float4 colour0 : SV_TARGET0;	
};

FS_OUT main( FS_IN input) {
	FS_OUT output;
	output.colour0 = input.col;

	return output;

}
