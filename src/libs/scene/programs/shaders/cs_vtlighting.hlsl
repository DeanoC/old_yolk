#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
#include "vtstructs.hlsl"

Texture2DMS<uint4,NUM_MSAA_SAMPLES> gBufferOpaque : register( t0 );
Texture2D<uint> fragmentCounter : register( t1 );
StructuredBuffer<VtTransparentFragment> transparentFragments : register( t2 );
Texture2DMS<float,NUM_MSAA_SAMPLES> depthBuffer : register( t8 );

StructuredBuffer<VtMaterial> materialStore : register( t10 );
StructuredBuffer<VtLight> lightStore : register( t11 );

RWTexture2D<float4> target : register( u0 );

void combSortTransparents( uint2 frags[TOTAL_TRANS_OR_AA_FRAGS], uint inputSize ) {
	uint gap = inputSize;
	bool swapped = false;
	while( gap > 1 && swapped == false ) {
		gap = uint((float)gap * (1.0f / 1.247330950103979f));
		if( gap < 1 ) {
			gap = 1;
		}

		uint i = 0;
		while( (i + gap) < inputSize ) {
			if( (frags[i].y & 0xFFFF) > (frags[i+gap].y & 0xFFFF) ) {
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
void shellSort( inout uint2 frags[TOTAL_TRANS_OR_AA_FRAGS], in uint inputSize ) {
	static const uint gaps[] = { 17, 9, 5, 3, 1 };
	[unroll] for( uint g = 0; g < 5; ++g ) {
		const uint gap = gaps[g];
		for( uint i = gap; i < inputSize; ++i ) {
			uint2 tmp = frags[i];
			uint j = i;
			while(	(j >= gap) && 
					(frags[j - gap].y & 0xFFFF) < (tmp.y & 0xFFFF) ) {
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
	float3 viewLightDir = mul( lightStore[lightIndex].position, matrixViewIT ).xyz;
	viewLightDir = normalize( viewLightDir ); // temp HACK !

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

void transparentFragment( in const float2 positionScreen, in const uint2 frag, inout float3 col[NUM_MSAA_SAMPLES] ) {
	uint matIndex = (frag.x >> 20) & 0xFFF;
	float depth = (frag.y & 0xFFFF) * (1.0f/65535.f);
	uint coverage = ((frag.y >> 24) & 0xFF);

	float2 esm = float2( (frag.x >> 10) & 0x3FF, (frag.x >> 0) & 0x3FF ) * (1/1023.f);
	float alpha = materialStore[ matIndex ].diffuse_transp.w;
	float3 viewNormal = decodeSphereMap( esm );

	float viewSpaceZ = matrixProjection._43 / (depth - matrixProjection._33);
	float3 viewPos = computePositionViewFromZ( positionScreen, viewSpaceZ );
	float3 litcol = lightFragment( viewNormal, viewPos, matIndex, 0 );

	uint count = 0;
	while( count < NUM_MSAA_SAMPLES ) {
		// TODO move first firstbithigh
		if( coverage & (1<<count) ) {
			col[count] = lerp( col[count], litcol, alpha );
		}
		count++;
	}
/*	uint sample = firstbitlow( coverage );
	while( sample < 0xFF ) {
		col[sample-1] = lerp( col[sample-1], litcol, alpha );
		coverage &= ~(1 << sample);
		sample = firstbitlow( coverage );
	}*/
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

    float3 col[NUM_MSAA_SAMPLES];
	
	// opaque phase
	{
		uint2 frags[NUM_MSAA_SAMPLES];
		float3 viewNormals[NUM_MSAA_SAMPLES];

		// read from all samples
		[unroll] for( uint sample = 0; sample < NUM_MSAA_SAMPLES; ++sample ) {
			frags[sample] = gBufferOpaque.Load( dispatchThreadId.xy, sample ).xy;
			float2 esm = float2( (frags[sample].x >> 10) & 0x3FF, (frags[sample].x >> 0) & 0x3FF ) * (1/1023.f);
			viewNormals[sample] = decodeSphereMap( esm );
		}
		// adaptively decide to do the rest
		bool perSampleLit = false;

		// if different materials, thats a definate
		static const float minNormalDot = 0.99f;        // Allow ~8 degree normal deviations
		uint matIndex = (frags[0].x >> 20) & 0xFFF;
		float3 viewNormal = viewNormals[0];

		[unroll] for( uint mi = 1; mi < NUM_MSAA_SAMPLES; ++mi ) {
			perSampleLit = perSampleLit || ( matIndex != ((frags[mi].x >> 20) & 0xFFF) );
			perSampleLit = perSampleLit || ( dot( viewNormal, viewNormals[mi] ) < minNormalDot );
		}

		// always do the first sample
		float viewSpaceZ = matrixProjection._43 / (depthBuffer.Load( dispatchThreadId.xy, 0 ) - matrixProjection._33);		
		float3 viewPos = computePositionViewFromZ( positionScreen, viewSpaceZ );

		col[0] = lightFragment( viewNormal, viewPos, matIndex, 0 );

		if( perSampleLit ) {
			for( uint psample = 1; psample < NUM_MSAA_SAMPLES; ++psample ) {
				uint2 frag = frags[psample].xy;

				uint matIndex = (frag.x >> 20) & 0xFFF;
				float3 viewNormal = viewNormals[psample];

				float viewSpaceZ = matrixProjection._43 / (depthBuffer.Load( dispatchThreadId.xy, psample ) - matrixProjection._33);
				float3 viewPos = computePositionViewFromZ( positionScreen, viewSpaceZ );

				col[psample] = lightFragment( viewNormal, viewPos, matIndex, 0 );
			}
		} else {
			for( uint psample = 1; psample < NUM_MSAA_SAMPLES; ++psample ) {
				col[psample] = col[0];
			}
		}
	}

	// transparent phase
	// how many transparent fragments do we have?
	uint fragCount = fragmentCounter[ dispatchThreadId.xy ];
	if( fragCount > 0 ) {
		uint index = ((dispatchThreadId.y * gbufferDim.x) + (dispatchThreadId.x) ) * TOTAL_TRANS_OR_AA_FRAGS;

		if( fragCount == 1 ) {
			uint2 frag =  uint2(	transparentFragments[ index ].matIndex_normal,
									transparentFragments[ index ].cov_depth );
			transparentFragment( positionScreen, frag, col );
		} else if( fragCount == 2 ){
			uint2 frag0 =  uint2(	transparentFragments[ index ].matIndex_normal,
									transparentFragments[ index ].cov_depth );
			uint2 frag1 =  uint2(	transparentFragments[ index + 1 ].matIndex_normal,
									transparentFragments[ index + 1 ].cov_depth );
			if( (frag1.y & 0xFFFF) > (frag0.y & 0xFFFF) ) {
				uint2 tmp = frag0;
				frag0 = frag1;
				frag1 = tmp;
			}
			transparentFragment( positionScreen, frag0, col );
			transparentFragment( positionScreen, frag1, col );
		} else {
			fragCount = min( fragCount, TOTAL_TRANS_OR_AA_FRAGS );
			// load the fragments 
			uint2 tfrags[ TOTAL_TRANS_OR_AA_FRAGS ];
			for( uint i = 0;i < fragCount; ++i ) {
				tfrags[ i ] =  uint2(	transparentFragments[ index + i ].matIndex_normal,
										transparentFragments[ index + i ].cov_depth );
			}

			shellSort( tfrags, fragCount );

			for( uint j = 0;j < fragCount; ++j ) {
				transparentFragment( positionScreen, tfrags[j], col );
			}
		}
	}
	float3 fcol = 0;
	[unroll] for( uint fsample = 0; fsample < NUM_MSAA_SAMPLES; ++fsample ) {
		fcol += col[fsample] * (1.0f / NUM_MSAA_SAMPLES);
	}

 	target[ dispatchThreadId.xy ] = float4( fcol, 1 );
}
