layout( points, invocations = 1 ) in;
layout( triangle_strip, max_vertices = 3 ) out;

layout( location = 0 ) smooth out vec2 outUV;

layout( binding = 0 ) uniform sampler2D tex;

void main() {
	vec2 texSize = vec2(1,1);//vec2( textureSize( tex, 0 ) );

    gl_Position = vec4( 0.0, 3.0, 0.0, 1.0 );
	outUV = vec2( 0.5, 2.0 ) * texSize;
    EmitVertex();

	gl_Position = vec4( 3.0, -1.0, 0.0, 1.0 );
	outUV = vec2( 2.0, -0.0 ) * texSize;
    EmitVertex();

    gl_Position = vec4( -3.0, -1.0, 0.0, 1.0 );
	outUV = vec2( -1.0, -0.0 ) * texSize;
    EmitVertex();

    EndPrimitive(); 
}