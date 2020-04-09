#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 tex_coord;

uniform mat4 camera_model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 camera_projection_matrix;

out vec3 normal;
out vec4 ecPos;
out vec2 texcoord;

void main () {
	texcoord = vec2(tex_coord.x, tex_coord.y);
	ecPos =  camera_model_view_matrix  * vec4 (vertex_position, 1.0);
	normal = normal_matrix * vertex_normal;
	gl_Position = camera_projection_matrix * ecPos;
}