#version 400
 
in vec3 normal;
in vec4 ecPos;
 
uniform vec3 L_pos;
uniform vec4 diffuse_col;

layout(location = 0) out vec4 colorOut;

 
void main()
{
	vec3 L = normalize(L_pos - vec3(ecPos)); // z fragmentu do svetla
	vec3 V = normalize(vec3(-ecPos)); //netreba delit s .w, lebo na toto este nie je aplikovana projekcna matica
 	vec3 N = normalize(normal);
	vec3 H = normalize(L + V); 
	
	float dif = clamp(dot(N, L), 0, 1);
	float spec = pow(dot(H, N), 100.0);
	
    colorOut = diffuse_col * dif + vec4(1.0, 1.0, 1.0, 1.0) * spec;
}