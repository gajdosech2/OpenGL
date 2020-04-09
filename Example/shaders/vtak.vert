#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 tex_coord;

uniform mat4 camera_model_view_matrix;
uniform mat3 normal_matrix;
uniform mat4 camera_projection_matrix;

uniform bool anim;
uniform float _t;

out VertexData {
    vec3 normal;
    vec4 ecPos;
    vec2 texcoord;
} VertexOut;


void main () {

	
	VertexOut.normal = normal_matrix * vertex_normal;    
	
	VertexOut.texcoord = tex_coord.xy;   
		
	//compute the vertex position  in camera space
	VertexOut.ecPos =  camera_model_view_matrix  * vec4 (vertex_position, 1.0);
	


	if (anim) {
		//float t = atan(2.5, abs(VertexOut.ecPos.x) );
		VertexOut.ecPos.xyz +=  abs(vertex_position.y) * vec3(0.0 , 0.6, 0.0) * (sin(2.0*_t) + sin(_t));//vec3(0,2.0,0)* sin(3.0 * t) * vertex_position.z ;
	}
	
	

	gl_Position = camera_projection_matrix * VertexOut.ecPos;

}