layout( location = 0 ) out vec4 outColour0;
layout( location = 1 ) smooth in vec2 inUV;

layout( binding = 0 ) uniform sampler2D counter;
layout( r32ui, binding = 0 ) uniform restrict uimageBuffer scratchMem;

void main() {
	// really an uint but encoded as a float to allow the 
	// blender to blend, always an integer value so okay to
	// chop it, fetch used to ensure no filtering
	vec4 count = texelFetch( counter, ivec2(inUV), 0 );

	// allocate a continuous block of size count
	uint allot = imageAtomicAdd( scratchMem, 0, uint(count.x) );

	// and store out to our head texture, each bin now has its
	// allocated space for all its fragments
	outColour0 = vec4( allot, 0, 0, 0 );
}
