#version 400
 
layout(triangles) in;
layout (line_strip, max_vertices=4) out;
 
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
  
  vec4 center = vec4(0.0, 0.0, 0.0, 0.0);
  
  for(int i = gl_in.length() - 1; i >= 0; i--)
  {
    center += gl_in[i].gl_Position;
    
     // copy attributes
    gl_Position = gl_in[i].gl_Position;
    VertexOut.normal = -VertexIn[i].normal;
    VertexOut.ecPos = VertexIn[i].ecPos;
 
    // done with the vertex
    EmitVertex(); 
    
  }
  
  center /= 3.0;
  
  vec3 v1 = gl_in[1].gl_Position.xyz/gl_in[1].gl_Position.w - gl_in[0].gl_Position.xyz/gl_in[0].gl_Position.w;
  vec3 v2 = gl_in[1].gl_Position.xyz/gl_in[1].gl_Position.w - gl_in[2].gl_Position.xyz/gl_in[2].gl_Position.w;
  
  vec3 v = normalize(cross(v1, v2)) * 0.5;

  vec3 d = center.xyz/center.w + v; 
  
  // set attributes
  gl_Position = vec4(d , 1.0);
  VertexOut.normal = vec3(0.0, 0.0, 1.0);
  VertexOut.ecPos = vec4(d , 1.0);
 
  // done with the vertex
  EmitVertex();
  
 
  EndPrimitive();
 
  
  
}