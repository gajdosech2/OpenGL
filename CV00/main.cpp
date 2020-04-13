#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <stdlib.h>
#include "GL/glew.c"
#include "GL/glut.h"
#include "tools/textfile.cpp"

GLuint v, f, p;

void printShaderInfoLog(GLuint obj) {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0) {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj) {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0) {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
        free(infoLog);
    }
}

void setShaders() {
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	char* vs = textFileRead((char*)"shaders/shader.vert");
	char* fs = textFileRead((char*)"shaders/shader.frag");

	const char* vv = vs;
	const char* fv = fs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &fv, NULL);

	free(vs); free(fs);	

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);

	p = glCreateProgram();
	glAttachShader(p, v);
	glAttachShader(p, f);

	glLinkProgram(p);
	printProgramInfoLog(p);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(p);

	glBegin(GL_TRIANGLES);
		glNormal3f(0.0, -1.0, 1.0);
		glVertex3f(-0.5, -0.5, 0.0);

		glNormal3f(1.0, 0.0, -1.0);
		glVertex3f(0.5, 0.0, 0.0);

		glNormal3f(0.0, 1.0, 1.0);
		glVertex3f(0.0, 0.5, 0.0);

		glNormal3f(0, -1.0, 1.0);
		glVertex3f(-0.5, -0.5, 0.0);

		glNormal3f(1.0, 0.0, -1.0);
		glVertex3f(0.5, 0.0, 0.0);

		glNormal3f(1.0, -1.0, 0.75);
		glVertex3f(0.5, -0.5, 0.0);
	glEnd();

    glutSwapBuffers();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 100);
	glutInitWindowSize(320, 320);
	glutCreateWindow("OpenGL");

	glewInit();
	if (!glewIsSupported("GL_VERSION_2_0")) {
		printf("OpenGL 2.0 not supported\n");
		return 1;
	}

	setShaders();

	glUseProgram(p);
	GLuint L = glGetUniformLocation(p, "L");
	glUniform3f(L, 0.0, 0.5, 1.0);

	glutDisplayFunc(renderScene);
	glutMainLoop();
	
	return 1;
}