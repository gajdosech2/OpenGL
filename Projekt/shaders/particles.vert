#version 400

layout(location = 0) in vec3 vertex_coord;
layout(location = 1) in vec4 particle_position_size;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out VertexData {
    vec4 ecPos;
    vec2 texCoord;
    float distance;
} VertexOut;


void main () {
	VertexOut.ecPos =  view_matrix * model_matrix * vec4(particle_position_size.xyz, 1.0);
	VertexOut.ecPos += vec4((vertex_coord - 0.5) * particle_position_size.w, 1.0); 
	VertexOut.texCoord = vertex_coord.xy;
	VertexOut.distance = length(vec3(particle_position_size));
	gl_Position = projection_matrix * VertexOut.ecPos;
}