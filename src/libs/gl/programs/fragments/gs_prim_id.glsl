layout( triangles, invocations = 1 ) in;
layout( triangle_strip, max_vertices = 3 ) out;

layout( r32i, binding = 2 ) uniform restrict iimageBuffer scratchMem;

layout( location = 0 ) in vec4 inNDCPosition[];

void main() {
	// primitive Id (TODO use atomic counter)
	gl_PrimitiveID = imageAtomicAdd( scratchMem, 2, 1 );

    gl_Position = inNDCPosition[0];
    EmitVertex();
    gl_Position = inNDCPosition[1];
    EmitVertex();
    gl_Position = inNDCPosition[2];
    EmitVertex();
}