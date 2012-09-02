struct VS_IN {
	float4 position 		: SV_POSITION;
	float3 worldPosition 	: TEXCOORD0;
};

struct GS_OUT {
	float4 position : SV_POSITION;
	float4 colour 	: COLOR0;
	float3 edgeH	: TEXCOORD0;
};

// Compute the triangle face normal from 3 points
float3 faceNormal( in float3 posA, in float3 posB, in float3 posC ) {
	return normalize( cross(normalize(posB - posA), normalize(posC - posA)) );
}

[maxvertexcount(3)]
void main( in triangle VS_IN input[3], inout TriangleStream<GS_OUT> stream0 ) {
	GS_OUT vert;

	vert.colour = float4( faceNormal(input[0].worldPosition, input[1].worldPosition, input[2].worldPosition), 1 );

	vert.position = input[0].position;
	vert.edgeH = float3( 1, 0, 0 );
	stream0.Append( vert );

	vert.position = input[1].position;
	vert.edgeH = float3( 0, 1, 0 );
	stream0.Append( vert );


	vert.position = input[2].position;
	vert.edgeH = float3( 0, 0, 1 );
	stream0.Append( vert );

	stream0.RestartStrip();
}
