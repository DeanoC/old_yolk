
struct VtMaterial {
	float4 diffuse; 	// w not used
	float4 emissive;	// w not used
	float4 specular;	// w = shininess AKA specular exponent
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