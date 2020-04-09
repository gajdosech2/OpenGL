#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 rotation4;
uniform mat3 rotation3;
uniform mat4 translation;
uniform float angle;

out VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexOut;

void main () {

mat3 m3 = rotation3;
mat4 m4 = rotation4;

float a = 0.0;

    //performs rotation by multiplication of rotation matrices about 0.1 degrees
    while(a < angle) { 

	m3 *= rotation3;
	m4 *= rotation4;
	
	a += 0.1;
    }
  

  VertexOut.normal = m3 * vertex_normal;

  //compute the vertex position  in camera space.
  VertexOut.ecPos = m4 * vec4 (vertex_position, 1.0);

  gl_Position = VertexOut.ecPos;
}