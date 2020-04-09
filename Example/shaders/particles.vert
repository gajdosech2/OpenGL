#version 400

layout(location = 0) in vec3 vertex_coord;
layout(location = 1) in vec4 particle_position_size;

uniform mat4 camera_model_view_matrix;
uniform mat4 camera_projection_matrix;

out VertexData {
    vec4 ecPos;
    vec2 texCoord;
    float distance;
} VertexOut;


void main () {
	//compute the vertex position  in camera space
	VertexOut.ecPos =  camera_model_view_matrix  *  vec4(particle_position_size.xyz, 1.0);
	VertexOut.ecPos += vec4 ((vertex_coord - 0.5)* particle_position_size.w, 1.0); 
	VertexOut.texCoord = vertex_coord.xy;
	VertexOut.distance = length(vec3(particle_position_size));//vzdialenost od emitera
	gl_Position = camera_projection_matrix * VertexOut.ecPos;

}