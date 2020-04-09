

void main()
{
	//vertex texture coordinate for texture unit 0
	gl_TexCoord[0] = gl_MultiTexCoord0;
    // Transforming The Vertex
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; 
}