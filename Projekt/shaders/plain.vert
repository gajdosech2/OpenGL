#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat3 normal_matrix;
uniform mat4 projection_matrix;

out VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexOut;

void main () {
	VertexOut.normal = normal_matrix * vertex_normal;
	VertexOut.ecPos = view_matrix * model_matrix * vec4(vertex_position, 1.0);
	gl_Position = projection_matrix * VertexOut.ecPos;
}