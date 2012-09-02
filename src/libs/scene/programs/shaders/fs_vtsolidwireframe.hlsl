struct FS_IN {
	float4 position : SV_POSITION;
	float4 colour 	: COLOR0;
	float3 edgeH	: TEXCOORD0;
};

float evalMinDistanceToEdges(in float3 edgeH) {
    float dist;

	float3 ddxHeights = ddx( edgeH );
	float3 ddyHeights = ddy( edgeH );
	float3 ddHeights2 =  (ddxHeights * ddxHeights) + (ddyHeights * ddyHeights);
	
    float3 pixHeights2 = (edgeH *  edgeH) / ddHeights2 ;
    
    dist = sqrt( min ( min (pixHeights2.x, pixHeights2.y), pixHeights2.z) );
    
    return dist;
}

static const float LineWidth = 1.5; // TODO add the cbuffer
static const float3 WireColour = float3(0.6,0.6,1); // TODO add the cbuffer
static const float FadeDistance = 50.f;

float4 main( FS_IN input ) : SV_Target {
    // Compute the shortest distance between the fragment and the edges.
    float dist = evalMinDistanceToEdges( input.edgeH );

    float4 col = input.colour;

    // Cull fragments too far from the edge.
    if (dist <= 0.5*LineWidth+1) {
		// Map the computed distance to the [0,2] range on the border of the line
		dist = clamp((dist - (0.5*LineWidth - 1)), 1e-5, 2);

		// Alpha is computed from the function exp2(-2(x)^2).
		float alpha = exp2( -2 * (dist*dist) );

		// Standard wire color but faded by distance
		// Dividing by pos.w, the depth in view space
		float fading = clamp(FadeDistance / input.position.w, 0, 1);

		col.xyz = lerp( col.xyz, WireColour, alpha * fading ) + alpha.xxx;
	}

	return col;
}
