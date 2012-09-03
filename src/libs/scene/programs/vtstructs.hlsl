
struct VtMaterial {
	float4 diffuse; 	// w not used
	float4 emissive;	// w not used
	float4 specular;	// w = shininess AKA specular exponent
};

struct VtDirectionalLight {
	float4	direction;	// w not used
	float4	colour;		// w/a not used
};