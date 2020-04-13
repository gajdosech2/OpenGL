varying vec3 N;
uniform vec3 L;
uniform vec4 farba;

void main()
{
	float diffuse = dot(normalize(N), normalize(L));
    gl_FragColor = vec4(1, 0, 0, 1) * diffuse;
}