#include "scenecapturestruct.glsl"

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
headCleaner( global uint* heads ) {
	const int id = (get_global_id(1) * get_global_size(0)) + get_global_id(0);
	heads[ id ] = 0xFFFFFFFF;
}

const sampler_t constant sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

kernel __attribute__ ((reqd_work_group_size (16,16,1))) void 
bitpackFragmentChain(	global uint*				heads,
						global struct FragmentLink* fragments,
						global unsigned long*		zOccupiedBits,
						read_only image2d_t			minMax ) {

	const int2 glbId = (int2)( get_global_id(0), get_global_id(1) );
	const int id = (get_global_id(1) * get_global_size(0)) + get_global_id(0);

	uint fragLink = heads[ id ];
	uint oldHead = fragLink;
	const float2 pixMinMax = read_imagef( minMax, sampler, glbId ).xw;
//	const float lineDepth = native_recip(pixMinMax.y - pixMinMax.x);
	const float lineDepth = 64.0f / (pixMinMax.y - pixMinMax.x);

	// the bit packed z dimensions is projective, so more bits are for the
	// near than far. currently 64 quantisation steps is used (testing)
	unsigned long fragBits = 0;

	// also sort the closest fragment to the first fragment, whilst
	// bit packing
	float closestDepth = MAXFLOAT;
	uint closestIndex = oldHead;
	uint closestPrevIndex = oldHead;
	uint prevFragLink = 0;
	
	while( fragLink != 0xFFFFFFFF ) {
		global const struct FragmentLink* fragment = fragments + fragLink;

		// looks like a nop, but isn't the left shift followed by the right shift (which are both arithmetic)
		// sign extends the 24 bit int into a 32 bit int correct for negative numbers
		const int iDepth = ((int)((fragment->fragment.depth&0x00FFFFFF) << 8)) >> 8;
		const float depth = ((float)iDepth) * (1.0f / (float)(1 << 23));

		const float p = (depth - pixMinMax.x) * lineDepth;
		const long bitPos = floor( p ); 
		fragBits |= 1UL << bitPos;

		if( depth > 0 && depth < closestDepth ) {
			closestDepth = depth;
			closestIndex = fragLink;
			closestPrevIndex = prevFragLink;
		}
		prevFragLink = fragLink;
		fragLink = fragment->nextIndex;
	}
	// store the occupancy bits
	zOccupiedBits[ id ] = fragBits;

	// put the closest fragmetn at the head
	// no need to be atomic, as each pixel is serviced by only one thread
	if( oldHead != closestIndex ) {
		// swap heads and relink
		heads[ id ] = closestIndex;
		fragments[closestPrevIndex].nextIndex = fragments[closestIndex].nextIndex;
		fragments[closestIndex].nextIndex = oldHead;
	}
}
