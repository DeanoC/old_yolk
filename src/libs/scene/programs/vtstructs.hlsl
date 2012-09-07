
struct VtMaterial {
	float4 diffuse_transp; 	// xyz = diffuse, w = transparency
	float4 emissive_transl;	// xyz = emissive, w = transluceny
	float4 specular;		// xyz = specular, w = shininess AKA specular exponent
	float4 reflection;		// x = reflection, yzw = not used
};

struct VtDirectionalLight {
	float4	direction;	// w not used
	float4	colour;		// w/a not used
};

// 1 32 bit unsigned integer
struct VtOpaqueFragment {
	uint matIndex_normal;		// 16 bit material index + encoded normal 2 8 bit 
};

// 2 32 bit unsigned integer
struct VtTransparentFragment {
	uint	matIndex_depth;		// 16 bit material index + 16 bit normalised depth
	uint	normal;				// encoded normal 2 8 bit, 16 bit spare
};
