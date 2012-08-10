layout( location = 0 ) out vec4 outColour0;
layout( location = 1 ) flat in vec4 inColour;

void main() {
	outColour0 = inColour;
}
