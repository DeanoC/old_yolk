
struct VtMaterial {
	float4 baseColour_Kd; 		// xyz = colour, w = diffuse
	float4 specExp_Ks_Kl_Kr;	// x = specular exponent, y = specular, z = luminosity, w = reflection factor
	float4 transp_transl;		// x = transparency, y = translucency, zw = unused
};

const static uint NUM_TRANS_FRAGS = 8;
const static uint NUM_MSAA_SAMPLES = 4;
const static uint TOTAL_TRANS_OR_AA_FRAGS = NUM_TRANS_FRAGS * NUM_MSAA_SAMPLES;

struct VtLight {
	float4	position;	// w == 1 currently world space
	float4	colour;		// w/a not used
};

// 1 32 bit unsigned integer
struct VtOpaqueFragment {
	uint matIndex_normal;		// 12 bit material index + encoded normal 2 10 bit 
};

// 2 32 bit unsigned integer
struct VtTransparentFragment {
	uint	matIndex_normal;	// 12 bit material index + encoded normal 2 10 bit 
	uint	cov_depth;		// 8 bit coverage count, 4 bit spare, 16 bit normalised depth (lower bits)
};
