#version 400
#pragma instanced

out vec4 frag_colour;
in vec3 color;
in vec2 textureCoordinate;
in vec3 vertex_out;
uniform sampler2D ourTexture;

void main() {
	float dot = dot(color, vec3(0.0, 0.0, 1.0));
	float mixing = 0.75;
	float distance = length(vertex_out);
	float blocks = 9.0;
	float grid_size = 40.0;
	float brightness = (abs(dot) * (1.0 - mixing) + mixing) * (1.0 - clamp(exp(0.003*(distance - grid_size*blocks)), 0.0, 1.0));
	frag_colour = mix(vec4(0.1, 0.1, 0.1, 1.0), texture(ourTexture, textureCoordinate), brightness);
}
