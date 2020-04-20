#version 400
 
layout(triangles) in;
layout (triangle_strip, max_vertices=6) out;
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexIn[3];
 
out VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexOut;
 
void main()
{
  for(int i = 0; i < gl_in.length(); i++)
  {
    gl_Position = gl_in[i].gl_Position;
    VertexOut.normal = -VertexIn[i].normal;
    VertexOut.ecPos = VertexIn[i].ecPos;
    EmitVertex(); 
  }
  
  vec3 center = (((gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz) / 2.0) + gl_in[2].gl_Position.xyz) / 2.0;
  
  vec3 v1 = gl_in[1].gl_Position.xyz/gl_in[1].gl_Position.w - gl_in[0].gl_Position.xyz/gl_in[0].gl_Position.w;
  vec3 v2 = gl_in[1].gl_Position.xyz/gl_in[1].gl_Position.w - gl_in[2].gl_Position.xyz/gl_in[2].gl_Position.w;
  vec3 v = normalize(cross(v1, v2));
  vec3 d = center + v; 

  gl_Position = vec4(d , 1.0);
  VertexOut.normal = vec3(0.0, 0.0, 1.0);
  VertexOut.ecPos = vec4(d , 1.0);
  EmitVertex();
  
  gl_Position = gl_in[0].gl_Position;
  VertexOut.normal = -VertexIn[0].normal;
  VertexOut.ecPos = VertexIn[0].ecPos;
  EmitVertex(); 
  
  gl_Position = gl_in[1].gl_Position;
  VertexOut.normal = -VertexIn[1].normal;
  VertexOut.ecPos = VertexIn[1].ecPos;
  Emi0tVertex(); 
 
  EndPrimitive();
}