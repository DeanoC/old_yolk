#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
#include "vtstructs.hlsl"
Texture2DMS<uint4,8> gBufferOpaque : register( t0 );
Texture2D<uint> fragmentCounter : register( t1 );
StructuredBuffer<VtTransparentFragment> transparentFragments : register( t2 );
Texture2DMS<float,8> depthBuffer : register( t8 );
const static int MAX_TRANS_FRAGS = 8;

StructuredBuffer<VtMaterial> materialStore : register( t10 );
StructuredBuffer<VtDirectionalLight> lightStore : register( t11 );

RWTexture2D<float4> target : register( u0 );

void combSortTransparents( uint2 frags[8], uint inputSize ) {
	uint gap = inputSize;
	bool swapped = false;
	while( gap > 1 && swapped == false ) {
		gap = uint((float)gap * (1.0f / 1.247330950103979f));
		if( gap < 1 ) {
			gap = 1;
		}

		uint i = 0;
		while( (i + gap) < inputSize ) {
			if( (frags[i].x & 0xFFFF) > (frags[i+gap].x & 0xFFFF) ) {
				uint2 tmp;
				tmp = frags[i];
				frags[i] = frags[i+gap];
				frags[i+gap] = tmp;
				swapped = true;
			}
			i++;
		}
	}
}
void shellSort( inout uint2 frags[8], in uint inputSize ) {
	static const uint gaps[] = { 7, 3, 1 };
	[unroll] for( uint g = 0; g < 3; ++g ) {
		const uint gap = gaps[g];
		for( uint i = gap; i < inputSize; ++i ) {
			uint2 tmp = frags[i];
			uint j = i;
			while(	(j >= gap) && 
					(frags[j - gap].x & 0xFFFF) > (tmp.x & 0xFFFF) ) {
				frags[j] = frags[j -gap]; 
				j -= gap;
			}
			frags[j] = tmp;
		}
	}
}

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

float3 lightFragment(	in const float3 viewNormal, 
						in const float3 viewPos,
						in const uint matIndex,
						in const uint lightIndex ) {
	float3 col;

	float3 difcol = materialStore[ matIndex ].diffuse_transp.xyz;
	float3 speccol = materialStore[ matIndex ].specular.xyz;
	float specpow = materialStore[ matIndex ].specular.w;
	float3 emmcol = materialStore[ matIndex ].emissive_transl.xyz;

	// emmisive doesn't care about dots etc.
	col = emmcol;

	// diffuse light needs view space light (TODO move to upload)
	float3 viewLightDir = mul(	float4(lightStore[lightIndex].direction.xyz,0), 
								matrixViewIT ).xyz;
	float3 lightCol = lightStore[lightIndex].colour.xyz; 

	float NdotL = dot( viewNormal, viewLightDir );
	if( NdotL > 0.0f ) {
		col += lightCol * difcol * max( 0.0f, NdotL ); 

		// specular if we are in the diffuse light part
		float3 viewDir = normalize( viewPos );
		float3 r = reflect( viewLightDir, viewNormal );
		float RdotV = max( 0.0f, dot( r, viewDir ) );
		col += lightCol * speccol * pow( RdotV, specpow ) * NdotL;
	}

	return col;
}

[numthreads(8,8,1)]
void main(    	uint3 groupId : SV_GroupID,
    			uint3 groupThreadId : SV_GroupThreadID,
    			uint3 dispatchThreadId : SV_DispatchThreadID,
   				uint groupIndex : SV_GroupIndex) {


 	float2 gbufferDim;
    uint dummy;
    gBufferOpaque.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);
    
    float2 screenPixelOffset = float2(2.0f, -2.0f) / gbufferDim;
    float2 positionScreen = (float2(dispatchThreadId.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);

    float3 col = 0;
	
	// opaque phase
	// TODO adaptive (we are super sampling at the moment ouchy!)
    for( uint sample = 0; sample < 8; ++sample ) {
		float depth = depthBuffer.Load( dispatchThreadId.xy, sample );
		uint2 frag0 = gBufferOpaque.Load( dispatchThreadId.xy, sample ).xy;

		uint matIndex = (frag0.x >> 16) & 0xFFFF;
		float2 esm = float2( (frag0.x >> 8) & 0xFF, (frag0.x >> 0) & 0xFF ) * (1/255.f);
		float3 viewNormal = decodeSphereMap( esm );

		float viewSpaceZ = matrixProjection._43 / (depth - matrixProjection._33);		
		float3 viewPos = computePositionViewFromZ( positionScreen, viewSpaceZ );

		col += lightFragment( viewNormal, viewPos, matIndex, 0 );

	}
	// take account of MSAA
	col /= 8;

	// transparent phase
	// how many transparent fragments do we have?
	uint fragCount = fragmentCounter[ dispatchThreadId.xy ];
	fragCount = min( fragCount, 8 );
	if( fragCount > 0 ) {
		// load the fragments 
		uint2 tfrags[ MAX_TRANS_FRAGS ];
		uint width;
		uint dummy0;
		fragmentCounter.GetDimensions( width, dummy0 );

		uint index = ((dispatchThreadId.y * width) + (dispatchThreadId.x) ) * MAX_TRANS_FRAGS;

		for( uint i = 0;i < fragCount; ++i ) {
			tfrags[ i ] =  uint2(	transparentFragments[ index + i ].matIndex_depth,
									transparentFragments[ index + i ].normal );
		}
		shellSort( tfrags, fragCount );

		for( uint j = 0;j < fragCount; ++j ) {
			uint2 frag = tfrags[j];
			uint matIndex = (frag.x >> 16) & 0xFFFF;
			float depth = (frag.x & 0xFFFF) * (1.0f/65535.f);

			float2 esm = float2( (frag.y >> 24) & 0xFF, (frag.y >> 16) & 0xFF ) * (1/255.f);
			float alpha = materialStore[ matIndex ].diffuse_transp.w;
			float3 viewNormal = decodeSphereMap( esm );

			float viewSpaceZ = matrixProjection._43 / (depth - matrixProjection._33);		
			float3 viewPos = computePositionViewFromZ( positionScreen, viewSpaceZ );

			col += lightFragment( viewNormal, viewPos, matIndex, 0 ) * alpha;
		}
	}

 	target[ dispatchThreadId.xy ] = float4( col, 1 );
}
