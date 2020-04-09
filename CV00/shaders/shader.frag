varying vec3 N;
uniform vec3 L;

void main()
{
	float diffuse = dot(normalize(N), normalize(L));
    gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0) * diffuse;
}