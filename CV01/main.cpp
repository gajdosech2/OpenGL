#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <stdlib.h>

#include "tools/textfile.cpp"

#include "GL/glew.c"
#include "GL/glut.h"
#include "tools/loadBMP.cpp"


GLuint v, f, p;

void printShaderInfoLog(GLuint obj) {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0) {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
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

	char *vs = textFileRead("shaders/shader.vert");
	char *fs = textFileRead("shaders/shader.frag");

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &ff, NULL);

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

void idle(void) {
	glutPostRedisplay();
}

GLuint texture;
void setTexture() {
	glEnable(GL_TEXTURE_2D);
	unsigned char * data = loadBMP("crate1_256x256.bmp");
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void renderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(0);

	glRotatef(0.1, 1.0, 0, 0); 
	glRotatef(0.2, 0, 1.0, 0);

	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, -1.0, 1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(1.0, -1.0, 1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 1.0, 1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, 1.0, 1.0);

		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(1.0, -1.0, -1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(-1.0, -1.0, -1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(-1.0, 1.0, -1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(1.0, 1.0, -1.0);

		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(1.0, -1.0, 1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(1.0, -1.0, -1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 1.0, -1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(1.0, 1.0, 1.0);

		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, -1.0, -1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(-1.0, -1.0, 1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(-1.0, 1.0, 1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, 1.0, -1.0);

		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, 1.0, 1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(1.0, 1.0, 1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 1.0, -1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, 1.0, -1.0);

		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, -1.0, -1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(1.0, -1.0, -1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, -1.0, 1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, -1.0, 1.0);
	glEnd();
	glutSwapBuffers();
}

void reshape(int w, int h) { 
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(40.0, (GLfloat) w/(GLfloat) h, 0.1, 100.0); 
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();
   glTranslatef (0.0, 0.0, -10.0);
}

GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};
GLfloat light_ambient[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(320, 320);
	glutCreateWindow("OpenGL");

	glewInit();
	if (!glewIsSupported("GL_VERSION_2_0")) {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}

	setShaders();

	glUseProgram(p);
	GLint unf_loc = glGetUniformLocation(p, "tex");
	glUniform1i(unf_loc, 0);

	glUseProgram(0);

	glutDisplayFunc(renderScene);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	setTexture();
	glutMainLoop();
	
	return 1;
}