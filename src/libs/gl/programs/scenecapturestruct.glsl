
// 96 bits = 12 bytes
struct Fragment {
	uint	depth;			// 24 bit depth + 8 bit coverage
	uint	primitiveId;	// 31 bit index + 1 bit facing
	uint	bary;			// 16 bit float for barycentric u and v (TODO ushort)
};

// 128 bits = 16 bytes
struct FragmentLink {
	struct Fragment	fragment;
	uint		nextIndex;
};

// 256bit bits = 32 bytes (.5 bytes free)
struct PrimitiveVertex {
	uint4				worldPos;		// w spare
	uint4				surf;			// x = world surface normal packHalf2x16(xy)(TODO short) 
										// y = 16 bit half normal.z + 15 bit materialId ( 17 spare bits)
										// z = packHalf2x16(uv)
										// w spare
};


struct Light {
	float4 	pos;				// center of light source in world 3 space
	float4	wo;					// if all zero, last light in the list, else normalised vector from light towards its focus
	float4	flux;				// Flux (total over a sphere) at RGB freq
	float	barnDoorDot;		// dot product * -1 of barn door cone (1 == sphere, 0 = hemisphere, -1 = infinitely thin along direction)
	float	dummy[3];
};