#version 400
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
    vec2 texcoord;
} VertexIn;
 
uniform vec3 L_pos;
uniform float L_inten;

uniform sampler2D uv_tex;//texturna jednotka s uv mapou

layout(location = 0) out vec4 fragmentOut;

 
void main()
{
  vec4 color = vec4(1,0,0,1);
   
  vec3 lightDir = normalize(L_pos - vec3(VertexIn.ecPos/VertexIn.ecPos.w)); 
  
  vec3 N = normalize(VertexIn.normal);
   
  vec4 dif_col = clamp(dot(lightDir, N), 0.0, 1.0) * vec4(0.7, 0.7, 0.9, 1.0) * L_inten;
  
  vec3 H = normalize(lightDir + N);
  
  vec4 spec_col = clamp(pow(dot(H, N), 30.0), 0.0, 1.0) * vec4(0.1, 0.1, 0.2, 1);
      
  fragmentOut = spec_col + (dif_col + vec4(0.4, 0.4, 0.5, 1.0)) * texture(uv_tex, VertexIn.texcoord).rgba;
}