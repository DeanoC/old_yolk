layout( location = 0 ) out vec4 outColour0;
layout( location = 1 ) smooth in vec2 inUV;

layout( binding = 0 ) uniform sampler2DMS msaa;

void main() {
	ivec2 uv = ivec2(inUV);
	vec4 colour = 	texelFetch( msaa, uv, 0 ) + 
					texelFetch( msaa, uv, 1 ) + 
					texelFetch( msaa, uv, 2 ) + 
					texelFetch( msaa, uv, 3 ) + 
					texelFetch( msaa, uv, 4 ) + 
					texelFetch( msaa, uv, 5 ) + 
					texelFetch( msaa, uv, 6 ) + 
					texelFetch( msaa, uv, 7 );
	outColour0 = colour * (1.0/8.0);
}
