#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 tex_coord;

uniform mat4 camera_model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 camera_projection_matrix;

uniform int pass_number;

out VertexData {
    vec3 normal;
    vec2 texcoord;
    vec4 ecPos;
} VertexOut;


void main () {
	
	
	if (pass_number == 2){
	
		VertexOut.normal = normal_matrix * vertex_normal; 
		VertexOut.texcoord = tex_coord.xy;   

	}
	
	//compute the vertex position  in camera space
	VertexOut.ecPos =  camera_model_view_matrix  * vec4 (vertex_position, 1.0);


	gl_Position = camera_projection_matrix * VertexOut.ecPos;


}