#version 400
#pragma instanced

out vec4 frag_colour;
in vec3 color;
in vec2 textureCoordinate;
uniform sampler2D ourTexture;

void main() {
	float dot = dot(color, vec3(0.0, 0.0, 1.0));
	float mixing = 0.75;
	dot = abs(dot) * (1.0 - mixing) + mixing;
	frag_colour = texture(ourTexture, textureCoordinate) * vec4(dot, dot, dot, 1.0);
}
