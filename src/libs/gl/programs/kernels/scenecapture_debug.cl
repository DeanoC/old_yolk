#include "clforgl.glsl"
#include "constant_blocks.glsl"
#include "scenecapturestruct.glsl"

const sampler_t constant sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
countFragmentChain( const global uint* heads,
					global const struct FragmentLink* fragments,
					write_only image2d_t dst ) {

	int2 glbId = (int2)( get_global_id(0), get_global_id(1) );
	const int id = (get_global_id(1) * get_global_size(0)) + get_global_id(0);

	uint fragLink = heads[ id ];

	int count = 0;
	
	while( fragLink != 0xFFFFFFFF ) {
		global const struct FragmentLink* fragment = fragments + fragLink;
		fragLink = fragment->nextIndex;
		count++;
	}

	float4 pix;
	pix.xyz =  (float)count / 255.f;
	pix.xyz *= 20.0f;
	pix.w = 1.0f;
	write_imagef( dst, glbId, pix );

}

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
countFragments( read_only image2d_t src, write_only image2d_t dst ) {
	int2 glbId;
	glbId.x = get_global_id(0);
	glbId.y = get_global_id(1);

	float4 pix = read_imagef( src, sampler, glbId ).x;
	pix.xyz *= 20.0f;
	pix.w = 1.0f;
	write_imagef( dst, glbId, pix );
}

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
debugZOccupiedBits( const global uint* zOccupiedBits,
					write_only image2d_t dst ) {

	const int2 glbId = (int2)( get_global_id(0), get_global_id(1) );
	const int id = (get_global_id(1) * get_global_size(0)) + get_global_id(0);

	uint bits = zOccupiedBits[ id ];

	float4 pix;
	pix.x = (float)((bits>>16) & 0xFF);
	pix.y = (float)((bits>>8) & 0xFF);
	pix.z = (float)((bits>>0) & 0xFF);
	pix.xyz *= native_recip( 255.f );
	pix.xyz *= 10.f;
	pix.w = 1.0f;
	write_imagef( dst, glbId, pix );
}

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
viewSpaceViewer( global const uint* heads,
							global const struct FragmentLink* fragments,
							constant const struct PerPipeline* perPipeline,
							write_only image2d_t dst ) {

	const int2 glbId = (int2)( get_global_id(0), get_global_id(1) );
	const int id = (get_global_id(1) * get_global_size(0)) + get_global_id(0);

	// view space pre divide
	// viewport correct = yc * 1/tan(fov/2)
	// viewport correct = xc * aspect * tan(fov/2)
	// far plane
	// (-zFar*xc, -zFar*yc, zFar)	 ___________ (zFar*xc, -zFar*yc, zFar)
	//								|			|
	//								|			|
	// (-zFar*xc, zFar*yc, zFar)	------------- (zFar*xc, zFar*yc, zFar)
	// near plane
	// (-zNear*xc, -zNear*yc, zNear)   ______  (zNear*xc, -zNear*yc, zNear)
	//								  |	     |
	//								  |	     |
	// (-zNear*xc, zNear*yc, zNear)	  -------- (zNear*xc, zNear*yc, zNear)

	// far view plane 
	const float3 fvpTL = (float3)(	-perPipeline->zPlanes.y * perPipeline->fov.x,
									-perPipeline->zPlanes.y * perPipeline->fov.y,
									perPipeline->zPlanes.y );
	const float3 fvpBR = (float3)(	perPipeline->zPlanes.y * perPipeline->fov.x,
									perPipeline->zPlanes.y * perPipeline->fov.y,
									perPipeline->zPlanes.y );
	// near view plane 
	const float3 nvpTL = (float3)(	-perPipeline->zPlanes.x * perPipeline->fov.x,
									-perPipeline->zPlanes.x * perPipeline->fov.y,
									perPipeline->zPlanes.x );
	const float3 nvpBR = (float3)(	perPipeline->zPlanes.x * perPipeline->fov.x,
									perPipeline->zPlanes.x * perPipeline->fov.y,
									perPipeline->zPlanes.x );
	// global id in (0..1] float coords
	const float2 fGId = (float2)(	(float)get_global_id(0) / (float)get_global_size(0),
									(float)get_global_id(1) / (float)get_global_size(1)  );

	const float3 ptAtFar = (float3)( mix( fvpTL.x, fvpBR.x, fGId.x ),
									 mix( fvpTL.y, fvpBR.y, fGId.y ),
									 fvpTL.z );
	const float3 ptAtNear = (float3)( mix( nvpTL.x, nvpBR.x, fGId.x ),
									 mix( nvpTL.y, nvpBR.y, fGId.y ),
									 nvpTL.z );

	const uint fragLink = heads[ id ];

	// head fragment is frontmost fragment
	if( fragLink == 0xFFFFFFFF ) {
		// nothing to hit, return sky or something?
		write_imagef( dst, glbId, (float4)(1.0f,1.f,0.0f,1.0f) );
		return;
	}

	global const struct FragmentLink* fragment = fragments + fragLink;
	const float depth = (float)fragment->fragment.depth * (1.0f / (float)(1 << 24));

	const float3 viewPt = mix( ptAtNear, ptAtFar, depth );

	float4 pix;
	pix.xyz = viewPt;
	pix.xyz *= native_recip( perPipeline->zPlanes.y );
	pix.xyz *= 10.f;
	pix.w = 1.0f;
	write_imagef( dst, glbId, pix );

}

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
displayBarycentrics(		global const uint* heads,							// too big to be a constant buffer
							global const struct FragmentLink* fragments,		// too big to be a constant buffer
							global const struct PrimitiveVertex* vertices,		// too big to be a constant buffer
							write_only image2d_t dst ) {

	const int id = (get_global_id(1) * get_global_size(0)) + get_global_id(0);
	const uint fragLink = heads[ id ];

	// head fragment is frontmost fragment
	if( fragLink == 0xFFFFFFFF ) {
		const int2 glbId = (int2)( get_global_id(0), get_global_id(1) );
		write_imagef( dst, glbId, (float4)(1.0f,1.f,0.0f,1.0f) );
		return;
	}

	global const struct FragmentLink* fragment = fragments + fragLink;

	uint3 uBary = (uint3)( (fragment->fragment.bary >> 16) & 0xFFFF, (fragment->fragment.bary >> 0) & 0xFFFF, 0xFFFF );
	uBary.z -= uBary.x + uBary.y;
	const float3 bary = convert_float3_rtz(uBary) * (1.f / 65535.f);

	// debug output
	float4 pix;
	pix.xyz = bary;
	pix.w = 1.0f;

	const int2 glbId = (int2)( get_global_id(0), get_global_id(1) );
	write_imagef( dst, glbId, pix );

}

/* NOTES
	// view space pre divide
	// viewport correct = yc * 1/tan(fov/2)
	// viewport correct = xc * aspect * tan(fov/2)
	// far plane
	// (-zFar*xc, -zFar*yc, zFar)	 ___________ (zFar*xc, -zFar*yc, zFar)
	//								|			|
	//								|			|
	// (-zFar*xc, zFar*yc, zFar)	------------- (zFar*xc, zFar*yc, zFar)
	// near plane
	// (-zNear*xc, -zNear*yc, zNear)   ______  (zNear*xc, -zNear*yc, zNear)
	//								  |	     |
	//								  |	     |
	// (-zNear*xc, zNear*yc, zNear)	  -------- (zNear*xc, zNear*yc, zNear)

	// far view plane 
	const float3 fvpTL = (float3)(	-perPipeline->zPlanes.y * perPipeline->fov.x,
									-perPipeline->zPlanes.y * perPipeline->fov.y,
									perPipeline->zPlanes.y );
	const float3 fvpBR = (float3)(	perPipeline->zPlanes.y * perPipeline->fov.x,
									perPipeline->zPlanes.y * perPipeline->fov.y,
									perPipeline->zPlanes.y );
	// near view plane 
	const float3 nvpTL = (float3)(	-perPipeline->zPlanes.x * perPipeline->fov.x,
									-perPipeline->zPlanes.x * perPipeline->fov.y,
									perPipeline->zPlanes.x );
	const float3 nvpBR = (float3)(	perPipeline->zPlanes.x * perPipeline->fov.x,
									perPipeline->zPlanes.x * perPipeline->fov.y,
									perPipeline->zPlanes.x );
	// global id in (0..1] float coords
	const float2 fGId = (float2)(	(float)get_global_id(0) / (float)get_global_size(0),
									(float)get_global_id(1) / (float)get_global_size(1)  );
	const float3 ptAtFar = (float3)( mix( fvpTL.x, fvpBR.x, fGId.x ),
									 mix( fvpTL.y, fvpBR.y, fGId.y ),
									 fvpTL.z );
	const float3 ptAtNear = (float3)( mix( nvpTL.x, nvpBR.x, fGId.x ),
									 mix( nvpTL.y, nvpBR.y, fGId.y ),
									 nvpTL.z );
 	const float depth = (float)(fragment->fragment.depth & 0x00FFFFFF) * (1.0f / (float)(1 << 24));
	const float3 viewPt = mix( ptAtNear, ptAtFar, depth );
 */

