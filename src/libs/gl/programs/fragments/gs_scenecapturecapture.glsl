#include "clforgl.glsl"
#include "scenecapturestruct.glsl"

// geometry program 'shape'
layout( triangles, invocations = 1 ) in;
layout( triangle_strip, max_vertices = 3 ) out;

// input from vertex stream (not all gl_PerVertex needs to be used)
in gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
} gl_in[];
layout( location = 0 ) in vec3 inWorldPosition[];
layout( location = 1 ) in vec3 inNormal[];
layout( location = 2 ) in vec2 inUv[];

// output 
out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};
layout(location = 0) out flat int primitiveId;
layout(location = 1) out smooth float3 barycentric;
layout(location = 2) out flat float wflip;
// buffer and texture inputs
layout( binding = 0, offset = 4 )	uniform atomic_uint primitiveCounter;
layout( rgba32ui, binding = 2 )		writeonly uniform uimage1D primitives; // each element == 1/2 PrimitiveVertex

void main() {
	primitiveId = int(atomicCounterIncrement( primitiveCounter ));
	PrimitiveVertex vertex;
	vertex.worldPos.w = 0; // Spare
	vertex.surf.w = 0; // Spare
	
	// same regardless of W flip and crossing
	vertex.worldPos.xyz = floatBitsToUint( inWorldPosition[0] );
	imageStore( primitives, (primitiveId*6)+0, vertex.worldPos );
	vertex.surf.x = packHalf2x16( inNormal[0].xy );
	vertex.surf.y = packHalf2x16( inNormal[0].zz );	// materialId TODO
	vertex.surf.z = packHalf2x16( inUv[0] );
	imageStore( primitives, (primitiveId*6)+1, vertex.surf );
	vertex.worldPos.xyz = floatBitsToUint( inWorldPosition[1] );
	imageStore( primitives, (primitiveId*6)+2, vertex.worldPos );
	vertex.surf.x = packHalf2x16( inNormal[1].xy );
	vertex.surf.y = packHalf2x16( inNormal[1].zz );	// materialId TODO
	vertex.surf.z = packHalf2x16( inUv[1] );
	imageStore( primitives, (primitiveId*6)+3, vertex.surf );
	vertex.worldPos.xyz = floatBitsToUint( inWorldPosition[2] );
	imageStore( primitives, (primitiveId*6)+4, vertex.worldPos );
	vertex.surf.x = packHalf2x16( inNormal[2].xy );
	vertex.surf.y = packHalf2x16( inNormal[2].zz );	// materialId TODO
	vertex.surf.z = packHalf2x16( inUv[2] );
	imageStore( primitives, (primitiveId*6)+5, vertex.surf );

	bool crossing = true;
	wflip = -1.0f;
	uint clipCode = 0;
	clipCode |= ( gl_in[0].gl_Position.w < 0 ) ? (1 << 0) : 0;
	clipCode |= ( gl_in[1].gl_Position.w < 0 ) ? (1 << 1) : 0;
	clipCode |= ( gl_in[2].gl_Position.w < 0 ) ? (1 << 2) : 0;
	switch( clipCode ) {
	case 0: // strictly positive
		wflip = 1.0f;
		crossing = false;
		break;
	case 7: // strictly negative
		crossing = false;
		break;
	default: // crossing in some way
		break;
	}

	// handle positive or negative and first crossing polygon
	gl_Position = gl_in[0].gl_Position;
	gl_Position.w *= wflip;
	barycentric = float3( 1, 0, 0 );
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	gl_Position.w *= wflip;
	barycentric = float3( 0, 1, 0 );
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	gl_Position.w *= wflip;
	barycentric = float3( 0, 0, 1 );
	EmitVertex();
	EndPrimitive();

	// handle crossing extra polygon
	if( crossing == true ) {
		wflip = 1.0f;
		gl_Position = gl_in[0].gl_Position;
		barycentric = float3( 1, 0, 0 );
		EmitVertex();
		gl_Position = gl_in[1].gl_Position;
		barycentric = float3( 0, 1, 0 );
		EmitVertex();
		gl_Position = gl_in[2].gl_Position;
		barycentric = float3( 0, 0, 1 );
		EmitVertex();
		EndPrimitive();
	}
}