layout( triangles, invocations = 1 ) in;
layout( triangle_strip, max_vertices = 3 ) out;

layout( r32ui, binding = 0 ) uniform restrict uimageBuffer scratchMem;
layout( rgba32f, binding = 1 ) uniform restrict writeonly imageBuffer faceBuffer;

// input from vertex stream (not all gl_PerVertex needs to be used)
in gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
} gl_in[];

layout( location = 1 ) in vec3 inWorldPosition[];

void main() {
	// allocate a continuous block of size count (TODO use atomic counter)
//	int allot = int( imageAtomicAdd( scratchMem, 1, 1 ) );

    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();

	// 3 vertices's per primitive
//	imageStore( faceBuffer, (allot * 3) + 0, vec4( inWorldPosition[0], 0 ) );
//	imageStore( faceBuffer, (allot * 3) + 1, vec4( inWorldPosition[1], 0 ) );
//	imageStore( faceBuffer, (allot * 3) + 2, vec4( inWorldPosition[2], 0 ) );
}
