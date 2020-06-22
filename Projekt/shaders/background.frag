uniform sampler2D color_tex;

void main() 
{
	gl_FragColor = vec4(0.6) * texture2D(color_tex, gl_TexCoord[0].st);
}