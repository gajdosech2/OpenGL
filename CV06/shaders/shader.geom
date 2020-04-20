#version 400
 
layout(triangles) in;
layout (triangle_strip, max_vertices=3) out;
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexIn[3];
 
out VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexOut;
 
void main() {
	for(int i = gl_in.length() - 1; i >= 0; i--) {
		// copy attributes
		gl_Position = gl_in[i].gl_Position;
		VertexOut.normal = VertexIn[i].normal;
		VertexOut.ecPos = VertexIn[i].ecPos;
 
		// done with the vertex
		EmitVertex();    
	}
	EndPrimitive();
}