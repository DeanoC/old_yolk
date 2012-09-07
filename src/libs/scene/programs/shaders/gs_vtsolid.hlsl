struct VS_IN {
	float4 position 		: SV_POSITION;
	float3 viewNormal		: NORMAL;
	float3 viewPosition 	: TEXCOORD0;
};

struct GS_OUT {
	float4 position : SV_POSITION;
	float3 viewNormal 	: TEXCOORD0;
};

// Compute the triangle face normal from 3 points
float3 faceNormal( in float3 posA, in float3 posB, in float3 posC ) {
	return normalize( cross(normalize(posB - posA), normalize(posC - posA)) );
}

static const float vertexNormalWeight = 1.5f;

[maxvertexcount(3)]
void main( in triangle VS_IN input[3], inout TriangleStream<GS_OUT> stream0 ) {
	GS_OUT vert;

	float3 faceViewNormal = faceNormal(input[0].viewPosition, input[1].viewPosition, input[2].viewPosition);

	vert.position = input[0].position;
	vert.viewNormal = normalize( faceViewNormal + (vertexNormalWeight * input[0].viewNormal) );
	stream0.Append( vert );

	vert.position = input[1].position;
	vert.viewNormal = normalize( faceViewNormal + (vertexNormalWeight * input[1].viewNormal) );
	stream0.Append( vert );

	vert.position = input[2].position;
	vert.viewNormal = normalize( faceViewNormal + (vertexNormalWeight * input[2].viewNormal) );
	stream0.Append( vert );

	stream0.RestartStrip();
}
