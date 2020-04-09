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

GLuint createShaderProgram2(char *vertex, char *tcontrol, char *tevaluation, char *geometry, char *fragment) {

    GLuint v, tc, te, g, f, p;


	char *vs = NULL,*tcs = NULL,*tes = NULL,*fs = NULL,*gs = NULL;

	// Create shader handlers
    v = glCreateShader(GL_VERTEX_SHADER);
	tc = glCreateShader(GL_TESS_CONTROL_SHADER);
	te = glCreateShader(GL_TESS_EVALUATION_SHADER);
    g = glCreateShader(GL_GEOMETRY_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER); 

	vs = textFileRead(vertex);
	tcs = textFileRead(tcontrol);
	tes = textFileRead(tevaluation);
	gs = textFileRead(geometry);
	fs = textFileRead(fragment);

	const char * vv = vs;
	const char * ttc = tcs;
	const char * tte = tes;
	const char * gg = gs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(tc, 1, &ttc, NULL);
	glShaderSource(te, 1, &tte, NULL);
	glShaderSource(g, 1, &gg, NULL);
	glShaderSource(f, 1, &ff, NULL);

	free(vs);free(tcs);free(tes);free(gs);free(fs);
 
    // Compile all shaders
    glCompileShader(v);
	glCompileShader(tc);
	glCompileShader(te);
    glCompileShader(g);
    glCompileShader(f);
 
    // Create the program
    p = glCreateProgram();
 
    // Attach shaders to program
    glAttachShader(p,v);
	glAttachShader(p,tc);
	glAttachShader(p,te);
    glAttachShader(p,g);
    glAttachShader(p,f);
 
    // Link and set program to use
    glLinkProgram(p);
    
	printProgramInfoLog(p);

	return p;

}

GLuint createShaderProgram1(char *vertex, char *geometry, char *fragment) {

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

GLuint createShaderProgram(char *vertex, char *fragment) {

    GLuint v,f,f2,p;


	char *vs = NULL,*fs = NULL,*fs2 = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	f2 = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead(vertex);
	fs = textFileRead(fragment);

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);
	printShaderInfoLog(f2);

	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);

	glLinkProgram(p);
	printProgramInfoLog(p);

	return p;

}