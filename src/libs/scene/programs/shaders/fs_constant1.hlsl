struct FS_OUT {
	float4 colour0 : SV_TARGET0;	
};

FS_OUT main() {
	FS_OUT output;

	output.colour0 = float4(1,1,1,1);

	return output;
}
