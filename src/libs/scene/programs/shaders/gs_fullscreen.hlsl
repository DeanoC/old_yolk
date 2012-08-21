
struct VS_IN {};

struct GS_OUT {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

[maxvertexcount(3)]
void main( in point VS_IN input[1], inout TriangleStream<GS_OUT> stream0 ) {

	GS_OUT vert;
	vert.position = float4( 0.0, 3.0, 0.0, 1.0 );
	vert.uv = float2( 0.5, 2.0 );
	stream0.Append( vert );

	vert.position = float4( 3.0, -1.0, 0.0, 1.0 );
	vert.uv = float2( 2.0, 0.0 );
	stream0.Append( vert );

	vert.position = float4( -3.0, -1.0, 0.0, 1.0 );
	vert.uv = float2( -1.0, 0.0 );
	stream0.Append( vert );

	stream0.RestartStrip();
}