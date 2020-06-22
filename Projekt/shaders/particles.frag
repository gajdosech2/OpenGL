#version 400
 
in VertexData {
    vec4 ecPos;
    vec2 texCoord;
    float distance;
} VertexIn;
 
layout(location = 0) out vec4 fragmentOut;

void main()
{     
  vec2 dr = VertexIn.texCoord - vec2(0.5, 0.5);
  float alpha = clamp(1 - 2 * length(dr), 0, 1);
  float fade = clamp(1 - VertexIn.distance / 7.0f, 0, 1);
  fragmentOut = vec4(alpha, 0.7, 0, alpha * fade);
}