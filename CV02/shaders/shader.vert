#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 rotation4;
uniform mat3 rotation3;
uniform float angle;

out vec3 normal;
out vec4 ecPos; //eye-coordinate position (pozicia v priestore kamery)

void main () {
	float a = 0;

	mat4 rot4 = rotation4;
	mat3 rot3 = rotation3;

	while(a < angle) {
		rot4 *= rotation4;
		rot3 *= rotation3;
		a += 0.1;
	}

	normal = rot3 * vertex_normal;
	ecPos = rot4 * vec4(vertex_position, 1.0);
	gl_Position = ecPos;
}