layout( points, invocations = 1 ) in;
layout( triangle_strip, max_vertices = 3 ) out;

layout( location = 1 ) smooth out vec2 outUV;

void main() {
    gl_Position = vec4( 0.0, 3.0, 0.0, 1.0 );
	outUV = vec2( 0.5, 2.0 );
    EmitVertex();

	gl_Position = vec4( 3.0, -1.0, 0.0, 1.0 );
	outUV = vec2( 2.0, -0.0 );
    EmitVertex();

    gl_Position = vec4( -3.0, -1.0, 0.0, 1.0 );
	outUV = vec2( -1.0, -0.0 );
    EmitVertex();

    EndPrimitive(); 
}