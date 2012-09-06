#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
#include "vtstructs.hlsl"

// normal encoding and decoding function from Andrew Lauritzen Intel Deferred Shading Sample
float3 decodeSphereMap(float2 e) {
    float2 tmp = e - e * e;
    float f = tmp.x + tmp.y;
    float m = sqrt(4.0f * f - 1.0f);
    
    float3 n;
    n.xy = m * (e * 4.0f - 2.0f);
    n.z  = 3.0f - 8.0f * f;
    return n;
}

// Position recovery from same sample as normal codec
float3 computePositionViewFromZ(float2 positionScreen, float viewSpaceZ) {
	// TODO replace divides by pre-stored recipricals
    float2 screenSpaceRay = float2(positionScreen.x / matrixProjection._11,
                                   positionScreen.y / matrixProjection._22);
    
    // Solve the two projection equations
    float3 positionView;
    positionView.xy = screenSpaceRay.xy * viewSpaceZ;
    positionView.z = viewSpaceZ;
    
    return positionView;
}


Texture2DMS<uint4,8> gBuffer0 : register( t0 );
Texture2DMS<float4,8> gBuffer1 : register( t1 );
Texture2DMS<float,8> depthBuffer : register( t9 );

StructuredBuffer<VtMaterial> materialStore : register( t10 );
StructuredBuffer<VtDirectionalLight> lightStore : register( t11 );

RWTexture2D<float4> target : register( u0 );

static const float LineWidth = 1.3f;
static const float FadeDistance = 50.f;

[numthreads(8,8,1)]
void main(    	uint3 groupId : SV_GroupID,
    			uint3 groupThreadId : SV_GroupThreadID,
    			uint3 dispatchThreadId : SV_DispatchThreadID,
   				uint groupIndex : SV_GroupIndex) {


 	float2 gbufferDim;
    uint dummy;
    gBuffer0.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);
    
    float2 screenPixelOffset = float2(2.0f, -2.0f) / gbufferDim;
    float2 positionScreen = (float2(dispatchThreadId.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);

    float3 col = 0; 
    for( uint sample = 0; sample < 8; ++sample ) {
		uint matIndex = gBuffer0.Load( dispatchThreadId.xy, sample ).x;
		uint coverage = gBuffer0.Load( dispatchThreadId.xy, sample ).y;
		float3 viewNormal = decodeSphereMap( gBuffer1.Load( dispatchThreadId.xy, sample ).xy );
		float edgeDist = gBuffer1.Load( dispatchThreadId.xy, sample ).z;
		float depth = depthBuffer.Load( dispatchThreadId.xy, sample );

		float viewSpaceZ = matrixProjection._43 / (depth - matrixProjection._33);		
		float3 viewPos = computePositionViewFromZ( positionScreen, viewSpaceZ );

		float3 difcol = materialStore[ matIndex ].diffuse_transp.xyz;
		float3 speccol = materialStore[ matIndex ].specular.xyz;
		float specpow = materialStore[ matIndex ].specular.w;
		float3 emmcol = materialStore[ matIndex ].emissive_transl.xyz;

		// emmisive doesn't care about dots etc.
		col += emmcol;

		// diffuse light needs view space light (TODO move to upload)
		float3 viewLightDir = mul( float4(lightStore[0].direction.xyz,0), matrixViewIT ).xyz;
		float3 lightCol = lightStore[0].colour.xyz; 
		float NdotL = dot( viewNormal, viewLightDir );
		if( NdotL > 0.0f ) {
			col += lightCol * difcol * max( 0.0f, NdotL ); 

			// specular if we are in the diffuse light part
			float3 viewDir = normalize( viewPos );
			float3 r = reflect( viewLightDir, viewNormal );
			float RdotV = max( 0.0f, dot( r, viewDir ) );
			col += lightCol * speccol * pow( RdotV, specpow ) * NdotL;
		}

		if (edgeDist <= 0.5*LineWidth+1) {
			// Map the computed distance to the [0,2] range on the border of the line
			float dist = clamp((edgeDist - (0.5*LineWidth - 1)), 0, 2);

			// Alpha is computed from the function exp2(-2(x)^2).
			float alpha = exp2( -2 * (dist*dist) );

			// Standard wire color but faded by distance
			// Dividing by pos.w, the depth in view space
			float fading = clamp(FadeDistance / viewSpaceZ, 0, 1);
			//		gBuffer.edgeAlpha = fading * alpha * 65535;
			col += col*alpha * fading;
		}
	}
 	target[ dispatchThreadId.xy ] = float4( col/8, 1 );
}
