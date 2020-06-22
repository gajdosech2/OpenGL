#version 400
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
	vec2 texcoord;
} VertexIn;
 
uniform vec3 L1_pos = vec3(0, -100, -100);
uniform vec4 L1_diffuse_col = vec4(1, 1, 1, 1);

uniform vec3 L2_pos = vec3(0, 100, 100);
uniform vec4 L2_diffuse_col = vec4(0.7, 0, 0.2, 1);

uniform sampler2D color_tex;

layout(location = 0) out vec4 fragmentOut;

void main() {
	vec4 dif_col = vec4(0,0,0,0);

	vec3 lightDir = normalize(vec3(L1_pos - VertexIn.ecPos.xyz));
    vec3 N = normalize(VertexIn.normal);
    float NdotL = max(dot(N, lightDir), 0.0);
    dif_col += L1_diffuse_col * NdotL;
	
	vec3 H = normalize(lightDir + N);
	vec4 spec_col = clamp(pow(dot(H, N), 10.0), 0.0, 1.0) * vec4(0.1, 0.1, 0.1, 1);
	
	lightDir = normalize(vec3(L2_pos - VertexIn.ecPos.xyz));
    N = normalize(VertexIn.normal);
    NdotL = max(dot(N, lightDir), 0.0);
    dif_col += L2_diffuse_col * NdotL;
	
	fragmentOut = spec_col + (vec4(0.1, 0.1, 0.1, 1.0) + dif_col) * texture(color_tex, VertexIn.texcoord).rgba;
}