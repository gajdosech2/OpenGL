#version 400
 
in vec3 normal;
in vec4 ecPos;
in vec2 texcoord;
 
uniform vec3 L_pos;
uniform sampler2D tex;

layout(location = 0) out vec4 colorOut;
 
void main()
{
  vec3 N = normalize(normal);
  vec3 L = normalize(L_pos - ecPos.xyz);
  vec3 V = normalize(-vec3(ecPos));
  vec3 H = normalize(L + V);
  float spec = clamp(pow(dot(N, H), 30.0), 0.0, 1.0);
  float NdotL = clamp(dot(N, L), 0.0, 1.0);
  colorOut = vec4(0.5,0.5,0.5,1) * spec + texture2D(tex, texcoord).rgba * NdotL + vec4(0.1, 0.1, 0.1, 1);
}