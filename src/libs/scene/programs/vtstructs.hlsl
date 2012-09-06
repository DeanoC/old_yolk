
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

// 4 16 bit unsigned integers
struct VtGBuffer0 {
	uint  materialIndex;
	uint  coverage;
	// 2x 16bit uint free
};

// 4 16 bit floats
struct VtGBuffer1 {
	float2  normal;
	float  	edgeDist; 
	// 1 half free
};

struct VtGbufferFragment {
	VtGBuffer0 frag0;
	VtGBuffer0 frag1;
};
