#include "clforgl.glsl"
#include "scenecapturestruct.glsl"
#include "constant_blocks.glsl"

// openCL 1.2 makes it standard and AMD have an extension
// NV supports in HW, so A PTX inline ASM would do it as well
// but here it in lovely slow C
/*int popcount(unsigned int x){
    int pop;
    for (pop = 0; x != 0; pop++)
        x &= x - 1;
    return pop;
}
*/
int popcount64(unsigned long x){
    int pop;
    for (pop = 0; x != 0; pop++)
        x &= x - 1;
    return pop;
}


float3 worldToRaster( const float3 wp, const float16 vpMatrix ) {
	float4 ppd;

	ppd.x = dot( wp, vpMatrix.s048 ) + vpMatrix.sc;
	ppd.y = dot( wp, vpMatrix.s159 ) + vpMatrix.sd;
	ppd.z = dot( wp, vpMatrix.s26a ) + vpMatrix.se;
	ppd.w = dot( wp, vpMatrix.s37b ) + vpMatrix.sf;

	ppd.xyz = ppd.xyz / ppd.w;
	ppd.xyz = (ppd.xyz * 0.5f) + 0.5f;

	ppd.x *= get_global_size(0); // assumes global size = raster width & height
	ppd.y *= get_global_size(1);

	return ppd.xyz;
}

int countCrossingsAheadOfPoint( const float3 rsP,
								const global unsigned long*			zOccupiedBits,
								read_only image2d_t			minMax ) {

	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

	const float2 pixMinMaxP = read_imagef( minMax, sampler, rsP.xy ).xw;
	if( rsP.z < pixMinMaxP.x ) {
		return 0;
	}

	int2 rsPi = (int2)convert_int2_rtz( rsP.xy );
	const float depth = rsP.z;

//	const float lineDepthP = native_recip(pixMinMaxP.y - pixMinMaxP.x);
	const float lineDepthP = 64.0f / (pixMinMaxP.y - pixMinMaxP.x);

	const float d = (depth - pixMinMaxP.x) * lineDepthP;
	const long bitPos = floor( d ); // 32 = bitlength of zoccupy
	const long bitMask = (1UL << bitPos) - 1UL;	// a power of two(pow2) - 1 == all bits set below the pow2 ((1 << n) == pow2)

	const long zOccupyP = zOccupiedBits[ (rsPi.y * get_image_width( minMax )) + rsPi.x ] & ~bitMask;

	return popcount64( zOccupyP );
}

// rsX = raster space point X
int pointToPointVisibility(	const float3 rsA, 
								const float3 rsB, 
								const global unsigned long*			zOccupiedBits,
								read_only image2d_t			minMax ) {
	int2 rsAi = (int2)convert_int2_rtz( rsA.xy );

	if( (rsAi.x < 0) || 
		(rsAi.y < 0) || 
		(rsAi.x >= get_image_width( minMax )) || 
		(rsAi.y >= get_image_height( minMax )) ) {
		return 1;
	}
	int steps = (int)length((rsB - rsA).xy);
	if( (steps*steps) >= (	get_image_width( minMax )*get_image_width( minMax ) + 
							get_image_height( minMax )*get_image_height( minMax )) ) {
		return 1;
	}


	int regionParity = countCrossingsAheadOfPoint( rsA, zOccupiedBits, minMax ) & 0x1;

	float3 rsP = rsA;

	float3 delta = (rsB - rsA) / steps;
	rsP += delta;
	steps--; // skip first hit
										
	for( int i=0;i < steps;i++ ) {
		int2 rsPi = (int2)convert_int2_rtz( rsP.xy );

		if( (rsPi.x < 0) || 
			(rsPi.y < 0) || 
			(rsPi.x >= get_image_width( minMax )) || 
			(rsPi.y >= get_image_height( minMax )) ) {
			return 1;
		}

		int regionParity2 = countCrossingsAheadOfPoint( rsP, zOccupiedBits, minMax ) & 0x1;
		if( regionParity != regionParity2 )
			return 0;
		rsP += delta;
	}

	return 1;
}
int countCrossingsAheadOfPoint2( const float3 rsP, 
								const global uint*				heads,
								const global struct FragmentLink* fragments,
								read_only image2d_t			minMax ) {

	int2 rsPi = (int2)convert_int2_rtz( rsP.xy );
	uint fragLink = heads[ (rsPi.y * get_image_width( minMax )) + rsPi.x ];
	if(fragLink == 0xFFFFFFFF ) return 0;
	int counter = 0;

	int iLinearDepth = convert_int_rtz(rsP.z * (float)(1 << 23));
	iLinearDepth -= 8000;

	// NOT SORTED YET
	while( fragLink != 0xFFFFFFFF ) {
		const global struct FragmentLink* fragment = fragments + fragLink;
		const int fragDepth = ((int)((fragment->fragment.depth&0x00FFFFFF) << 8)) >> 8;

		if( fragDepth < iLinearDepth ) {
			counter++;
		}

		fragLink = fragment->nextIndex;
	}

	return counter;
}

int pointToPointVisibility2(	const float3 rsA, 
								const float3 rsB, 
								const global uint*				heads,
								const global struct FragmentLink* fragments,
								read_only image2d_t			minMax ) {

	int2 rsAi = (int2)convert_int2_rtz( rsA.xy );

	if( (rsAi.x < 0) || 
		(rsAi.y < 0) || 
		(rsAi.x >= get_image_width( minMax )) || 
		(rsAi.y >= get_image_height( minMax )) ) {
		return 1;
	}
	int steps = (int)length((rsB - rsA).xy);
	if( (steps*steps) >= (	get_image_width( minMax )*get_image_width( minMax ) + 
							get_image_height( minMax )*get_image_height( minMax )) ) {
		return 1;
	}


	int regionParity = countCrossingsAheadOfPoint2( rsA, heads, fragments, minMax ) & 0x1;

	float3 rsP = rsA;

	float3 delta = (rsB - rsA) / steps;
	rsP += delta;
	steps--; // skip first hit

	for( int i=0; i < steps; i++ ) {
		int2 rsPi = (int2)convert_int2_rtz( rsP.xy );

		if( (rsPi.x < 0) || 
			(rsPi.y < 0) || 
			(rsPi.x >= get_image_width( minMax )) || 
			(rsPi.y >= get_image_height( minMax )) ) {
			return 1;
		}

		int regionParity2 = countCrossingsAheadOfPoint2( rsP, heads, fragments, minMax ) & 0x1;
		if( regionParity != regionParity2 )
			return 0;

		rsP += delta;
	}

	return 1;
}



int getGlobalId() {
	return (get_global_id(1) * get_global_size(0)) + get_global_id(0);
}
int2 getGlobal2DId() {
	return (int2)( get_global_id(0), get_global_id(1) );
}

uint checkHitAnything( global const uint* heads, write_only image2d_t dst ) {
	const uint fragLink = heads[ getGlobalId() ];

	// head fragment is frontmost fragment
	if( fragLink == 0xFFFFFFFF ) {
		// nothing to hit, return sky or something?
		const int2 glbId = (int2)( get_global_id(0), get_global_id(1) );
		write_imagef( dst, glbId, (float4)(1.0f,1.f,0.0f,1.0f) );
	}

	return fragLink;
}

float3 getBarycentricCoords( global const struct FragmentLink* fragment ) {
	global const half* hBary = (global const half*) fragment;
	float3 bary = (float3)(vload_half2( 2, hBary), 1.0f );
	bary.z = 1.0f - bary.y - bary.x;

	return bary;
}

float3 getWorldPosition( global const struct PrimitiveVertex* v, const float3 bary ) {
	global const half* hVerts = ((global const half*) v) + 8;

	// calc world position
	const float3 pos =	(bary.x * as_float3(v[0].worldPos.xyz)) + 
						(bary.y * as_float3(v[1].worldPos.xyz)) + 
						(bary.z * as_float3(v[2].worldPos.xyz));
	return pos;
}

float3 getWorldNormal( global const struct PrimitiveVertex* v, const float3 bary ) {
	global const half* hVerts = ((global const half*) v) + 8;

	// calc normal
	const float3 n0 = vload_half3( 0, hVerts+0 );
	const float3 n1 = vload_half3( 0, hVerts+16 );
	const float3 n2 = vload_half3( 0, hVerts+32 );

	const float3 normal = (bary.x * n0) + (bary.y * n1) + (bary.z * n2);
	return normal;
}

float3 lightDirectNoVis( 	const float3 pos,
							const float3 normal,
							constant const struct Light* curLight ) {
	const float d = fast_distance( curLight->pos.xyz, pos );
	const float3 w_p2l = normalize( curLight->pos.xyz - pos );
	const float ndotw_p2l = dot( w_p2l, normal );
	const float bdd = dot( curLight->wo.xyz, w_p2l );		// procedural barn doors for spotlight, hemi and sphere (and mixtures)

	float3 v = max( ndotw_p2l, 0.0f );						// surface reflection is over a hemisphere
	v *= ( bdd < curLight->barnDoorDot ) ? 1 : 0;			// in barnDoorDot solid angles else light is stopped
	v *= curLight->flux.xyz * (1.0f / (4.0f * (float)M_PI));		//  flux (power) / infinitismal area
	v *= native_recip( d );								// attenuate 1/d*d TODO atmosphere
	v *= 0.4f;	// fixed Kd
	return v;
}

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
testlighting_novis_0bounce( global const uint*							heads,							
							global const struct FragmentLink*			fragments,		
							global const struct PrimitiveVertex*		vertices,		
							constant const struct Light*				lights,	
							write_only image2d_t						dst ) {

	const uint fragLink = checkHitAnything( heads, dst );
	if( fragLink == 0xFFFFFFFF ) return;

	// get fragment and primtive's vertices
	global const struct FragmentLink* fragment = fragments + fragLink;
	global const struct PrimitiveVertex* v = vertices + ((fragment->fragment.primitiveId & ~(1<<31)) * 3);

	// calculate fragment parameters
	const float3 bary = getBarycentricCoords( fragment );
	const float3 pos = getWorldPosition( v, bary );
	const float3 normal = getWorldNormal( v, bary );

	// for each light
	constant const struct Light* curLight = lights;

	float3 colour = 0;
	// a light of direction 0,0,0 is the last in the list
	while( dot(curLight->wo.xyz, curLight->wo.xyz) > 1e-3f ) {
		colour.xyz += lightDirectNoVis( pos, normal, curLight );
		curLight++;
	}

	// debug output
	write_imagef( dst, getGlobal2DId(), (float4)( colour, 1.0f ) );
}

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
testlighting_0bounce(		global const uint*							heads,
							global const struct FragmentLink*			fragments,
							global const struct PrimitiveVertex*		vertices,
							constant const struct Light*				lights,
							global const unsigned long*							zOccupiedBits,
							read_only image2d_t							minMax,
							constant const struct PerViews*				perViewConstants,
							write_only image2d_t						dst ) {

	const uint fragLink = checkHitAnything( heads, dst );
	if( fragLink == 0xFFFFFFFF ) return;

	// get fragment and primtive's vertices
	global const struct FragmentLink* fragment = fragments + fragLink;
	global const struct PrimitiveVertex* v = vertices + (fragment->fragment.primitiveId * 3);

	// calculate fragment parameters
	const float3 bary = getBarycentricCoords( fragment );
	const float3 pos = getWorldPosition( v, bary );
	const float3 normal = getWorldNormal( v, bary );

	const float3 rasterPos = worldToRaster( pos, perViewConstants->matrixViewProjection );

	// for each light
	constant const struct Light* curLight = lights;

	float3 colour = 0;
	// a light of direction 0,0,0 is the last in the list
	while( dot(curLight->wo.xyz, curLight->wo.xyz) > 1e-3f ) {
//		bool vis = pointToPointVisibility(	rasterPos, 
//											worldToRaster( curLight->pos.xyz, perViewConstants->matrixViewProjection ),
//											zOccupiedBits,
//											minMax ); 
		float3 lc = lightDirectNoVis( pos, normal, curLight );// (float3)(1,1,1);//

		if( dot(lc,lc) > 1e-3f ) {
//			lc *= pointToPointVisibility( rasterPos, worldToRaster( curLight->pos.xyz, perViewConstants->matrixViewProjection ), zOccupiedBits, minMax ); 
			lc *= pointToPointVisibility2( rasterPos, worldToRaster( curLight->pos.xyz, perViewConstants->matrixViewProjection ), heads, fragments, minMax ); 
		}
		colour.xyz += lc;

		curLight++;
	}

//	colour.xy = rasterPos.xy - (float2)( get_global_id(0), get_global_id(1) );
//	colour *= (1.0f / 8.f);
//	colour *= (1.0f / 256.f);
//	colour = rasterPos * (1.0f/640.f);
//	colour = (colour * 0.5f) + 0.5f;

	// debug output
	write_imagef( dst, getGlobal2DId(), (float4)( colour, 1.0f ) );
}
