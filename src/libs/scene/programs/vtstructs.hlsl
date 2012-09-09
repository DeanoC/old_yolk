
struct VtMaterial {
	float4 diffuse_transp; 	// xyz = diffuse, w = transparency
	float4 emissive_transl;	// xyz = emissive, w = transluceny
	float4 specular;		// xyz = specular, w = shininess AKA specular exponent
	float4 reflection;		// x = reflection, yzw = not used
};

const static uint NUM_TRANS_FRAGS = 8;
const static uint NUM_MSAA_SAMPLES = 4;
const static uint TOTAL_TRANS_OR_AA_FRAGS = NUM_TRANS_FRAGS * NUM_MSAA_SAMPLES;

struct VtLight {
	float4	position;	// w == 1 currently world space
	float4	colour;		// w/a not used
	float 	attenuationEnd;
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
