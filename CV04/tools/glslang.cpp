/*
 OpenGL Shading Languge 
 http://www.lighthouse3d.com/opengl/glsl/
*/

#include <stdio.h>
#include <stdlib.h>
#include "glslang.h"

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

char *textFileRead(char *fn) {


	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fopen_s(&fp, fn,"rt");

	if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

GLuint createShaderProgram(char *vertex, char *geometry, char *fragment) {

    GLuint v, g, f, p;


	char *vs = NULL,*fs = NULL,*gs = NULL;

	// Create shader handlers
    v = glCreateShader(GL_VERTEX_SHADER);
    g = glCreateShader(GL_GEOMETRY_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER); 

	vs = textFileRead(vertex);
	gs = textFileRead(geometry);
	fs = textFileRead(fragment);

	const char * vv = vs;
	const char * gg = gs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(g, 1, &gg, NULL);
	glShaderSource(f, 1, &ff, NULL);

	free(vs);free(gs);free(fs);
 
    // Compile all shaders
    glCompileShader(v);
    glCompileShader(g);
    glCompileShader(f);
 
    printShaderInfoLog(v);
    printShaderInfoLog(g);
    printShaderInfoLog(f);
 
    // Create the program
    p = glCreateProgram();
 
    // Attach shaders to program
    glAttachShader(p,v);
    glAttachShader(p,g);
    glAttachShader(p,f);
 
    // Link and set program to use
    glLinkProgram(p);
    
    printProgramInfoLog(p);

	return p;

}