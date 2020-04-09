#version 400
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexIn;
 
uniform vec3 L_pos;
uniform vec4 diffuse_col;

layout(location = 0) out vec4 colorOut;

 
void main()
{
    // Compute the ligt direction
    vec3 lightDir = vec3(L_pos - VertexIn.ecPos.xyz);
    
    // Compute the dot product between normal and lightDir 
    float NdotL = max(dot(normalize(VertexIn.normal), normalize(lightDir)), 0.0);
    
    // Setting Each Pixel To Red
    colorOut = diffuse_col * NdotL;
}