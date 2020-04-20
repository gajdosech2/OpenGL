#include <iostream>
#include <stdlib.h>
#include "GL/glew.c"
#include "GL/glut.h"
#include "tools/glslang.cpp"

#define PI 3.14159265

GLuint p;  //shader program
GLuint vao; //Vertex Array Object
float angle = 0.0f;  //rotation angle

void setVertexArray() {

	// Data for a set of triangles
	float points[] = {
	-0.5f, -0.5f,  0.5f,
	0.0f, -0.5f,  -0.5f,
	0.5f, -0.5f,  0.5f};

	float normals[] = {
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f};

	// vertex buffer objects 
	GLuint points_vbo = 0;
	glGenBuffers (1, &points_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
	glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), points, GL_STATIC_DRAW);

	GLuint normals_vbo = 0;
	glGenBuffers (1, &normals_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, normals_vbo);
	glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), normals, GL_STATIC_DRAW);

	//Vertex Array Object
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void renderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(p);

	int loc = glGetUniformLocation(p, "angle"); //set rotation angle
	glUniform1f(loc, angle);

	glBindVertexArray (vao);
	// draw points 0-3 from the currently bound VAO with current in-use shader
	glDrawArrays (GL_TRIANGLES, 0, 3);

	angle += 0.25f;

    glutSwapBuffers();
}

int main(int argc, char **argv) {

	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(640,640);
	glutCreateWindow("OpenGL");

	// init GLEW to support extensions
	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
        printf("Ready for OpenGL 3.3\n");
    else {
        printf("OpenGL 3.3 not supported\n");
        exit(1);
    }

	p = createShaderProgram((char *)"./shaders/shader.vert", (char *)"./shaders/shader.geom", (char *)"./shaders/shader.frag");

	glUseProgram(p);

	//This is the location of the L_pos uniform variable in our program
	int loc = glGetUniformLocation(p, "L_pos");
	glUniform3f(loc, 0.0, 0.0, 10.0);

	//This is the location of the L_pos uniform variable in our program
	loc = glGetUniformLocation(p, "diffuse_col");
	glUniform4f(loc, 1.0, 1.0, 0.0, 1.0);
	
	float c = cos(0.1*PI/180.0);
	float s = sin(0.1*PI/180.0);

	// set matrix with an array
	float m1[] = {c*c, -s, c*s, 0, 
				  c*s, c, s*s, 0, 
				   -s, 0, c, 0, 
					0, 0, 0, 1};
	float m2[] = {c*c, -s, c*s, 
				  c*s, c, s*s,
				  -s, 0, c};

	float translation[] = {1, 0, 0, 0, 
						0, 1, 0, 0, 
						0, 0, 1, 10, 
						0, 0, 0, 1};

	//This is the location of the rotation4 uniform variable in our program
	loc = glGetUniformLocation(p, "rotation4");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  m1);

	//This is the location of the rotation3 uniform variable in our program
	loc = glGetUniformLocation(p, "rotation3");
	glUniformMatrix3fv(loc, 1, GL_FALSE,  m2);

	//This is the location of the translation uniform variable in our program
	loc = glGetUniformLocation(p, "translation");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  translation);

	setVertexArray();

	// register callbacks
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);

	glEnable(GL_DEPTH_TEST);

	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;
}