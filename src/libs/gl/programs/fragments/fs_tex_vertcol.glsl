layout( location = 0 ) out vec4 outColour0;
layout( location = 1 ) smooth in vec4 inColour;
layout( location = 2 ) smooth in vec2 inUV;

layout( binding = 0 ) uniform sampler2D tex;

void main() {
//	vec4 texColour = vec4(texture( tex, inUV ).b, 0, 0, 0) * 16.f;//vec4(inUV,0,0);
	vec4 texColour = texture( tex, inUV );
	outColour0 = inColour - texColour;
}
