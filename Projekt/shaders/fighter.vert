#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat3 normal_matrix;
uniform mat4 projection_matrix;

uniform int pass_number;

out VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexOut;

void main () {

	if (pass_number == 1) {
        VertexOut.normal = normal_matrix * vertex_normal;
		VertexOut.ecPos = view_matrix * model_matrix * vec4(vertex_position, 1.0);
		gl_Position = projection_matrix * VertexOut.ecPos;
	}
	
	if (pass_number == 2) {
		VertexOut.ecPos = vec4(vertex_position, 1.0);
		gl_Position = VertexOut.ecPos;
    }

}