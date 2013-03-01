
layout( location = 0 ) in smooth vec2 inUvs;
layout( location = 0 ) out vec4 outColour0;

layout( binding = 0 ) uniform sampler2D counter;
//layout( binding = 1 ) uniform usampler2D fragHeader;
//layout( binding = 2 ) uniform usamplerBuffer fragBuffer;

void main() {
	ivec2 uvs = ivec2( inUvs * 1024 );
//	uint index = texelFetch( fragHeader, uvs, 0 ).x;
//	uint count = uint( texelFetch( counter, uvs, 0).x * 100.0);

//	outColour0 = vec4(texelFetch( counter, uvs, 0).x) * 10;
	outColour0.r = texture( counter, inUvs ).r;
//	outColour0.r = texelFetch( counter, uvs, 0 ).r;
	outColour0.gba = vec3(inUvs, 0);

//	for( uint i = 0;i < count; ++i ) {
//	uint i = 0;
//		uvec4 fragment = texelFetch( fragBuffer, int(index + i) );
//		outColour0 = vec4(fragment);
//	}

}
