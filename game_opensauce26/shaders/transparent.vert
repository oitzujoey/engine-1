#version 400

layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
uniform vec4 orientation;
uniform vec3 position;
uniform float aspectRatio;
uniform float scale;
out vec3 color;
out vec2 textureCoordinate;

vec4 conjugate(vec4 a) {
	return vec4(-a.x,
	            -a.y,
	            -a.z,
	            a.w);
}

vec4 hamilton(vec4 a, vec4 b) {
	return vec4(a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
	            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
	            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
	            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);
}

vec3 rotate(vec3 v, vec4 q) {
	vec4 v4 = vec4(v, 0);
	v4 = hamilton(hamilton(q, v4), conjugate(q));
	return vec3(v4.x, v4.y, v4.z);
}

float w = 11.0*aspectRatio;
float h = 11.0;
float n = 11.0;
float f = 7500.0;
mat4 projectionMatrix = mat4(2.0*n/w, 0.0, 0.0, 0.0,
                             0.0, 2.0*n/h, 0.0, 0.0,
                             0.0, 0.0, -(f+n)/(f-n), -1.0,
                             0.0, 0.0, -2.0*f*n/(f-n), 0.0);

void main() {
	vec3 vertex = rotate(scale * vp, orientation);
	vertex += position;
	gl_Position = projectionMatrix * vec4(vertex, 1.0);

	float powerFactor = 1.01;
	float mixing = 0.5;
	color = (pow(powerFactor, -abs(gl_Position.z))/powerFactor * (1.0 - mixing) + mixing) * rotate(normal, orientation);
	textureCoordinate = texCoord;
}
