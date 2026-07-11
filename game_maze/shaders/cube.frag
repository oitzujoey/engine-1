#version 400
#pragma instanced

out vec4 frag_color;
in vec3 color;
in vec2 textureCoordinate;
in vec3 vertex_out;
uniform sampler2D ourTexture;

void main() {
	float mixing = 0.75;
	float blocks = 13.0;
	float grid_size = 40.0;
	float distance_max = blocks*grid_size;

	float dot = dot(color, vec3(0.0, 0.0, 1.0));
	float distance = length(vertex_out);
	float flashlight_mixing = 0.9;
	float flashlight = 1.45*(flashlight_mixing/(1.0 + exp(-5.0*length(vec2(vertex_out.z, vertex_out.z))/length(vec2(vertex_out.x, vertex_out.y)) + 10.0)) + (1.0 - flashlight_mixing))*(distance_max-distance)/distance_max;
	float remainder = max(flashlight - 1.0, 0.0);
	/* distance /= flashlight; */
	float brightness = (abs(dot) * (1.0 - mixing) + mixing) * (1.0 - clamp(exp((distance - distance_max)), 0.0, 1.0))*flashlight;
	vec4 f_color = texture(ourTexture, textureCoordinate);
	f_color.rgb = 1.0 - exp(-(f_color.rgb + vec3(remainder)));
	frag_color = mix(vec4(0.1, 0.1, 0.1, 1.0), f_color, brightness);
}
