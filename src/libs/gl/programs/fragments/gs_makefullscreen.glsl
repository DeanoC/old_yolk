layout( points, invocations = 1 ) in;
layout( triangle_strip, max_vertices = 4 ) out;

layout( location = 1 ) smooth out vec2 outUV;

layout( binding = 0 ) uniform sampler2DMS msaa;

void main() {
	vec2 msaaSize = vec2( textureSize( msaa ) );

    gl_Position = vec4(-1.0, 1.0, 0.5, 1.0 );
	outUV = vec2( 0.0, 1.0 );
	outUV = vec2( msaaSize *outUV );
    EmitVertex();

	gl_Position = vec4( 1.0, 1.0, 0.5, 1.0 );
	outUV = vec2( 1.0, 1.0 );
	outUV = vec2( msaaSize *outUV );
    EmitVertex();

    gl_Position = vec4(-1.0,-1.0, 0.5, 1.0 );
	outUV = vec2( 0.0, 0.0 );
	outUV = vec2( msaaSize *outUV );
    EmitVertex();

    gl_Position = vec4( 1.0,-1.0, 0.5, 1.0 );
	outUV = vec2( 1.0, 0.0 );
	outUV = vec2( msaaSize *outUV );
    EmitVertex();

    EndPrimitive(); 
}