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
	-0.5f, -0.5f,  0.0f,
	0.5f, 0.0f,  0.0f,
	0.0f, 0.5f,  0.0f };

	//ToDo definujte pole normal
	float normals[] = {
		0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f
	};

	// vertex buffer objects 
	//ToDo vytvorte VBO
	GLuint points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	//ToDo vytvorte VBO
	GLuint normals_vbo = 0;
	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	//Vertex Array Object
	//ToDo priradte vytvorene objekty k VAO 
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0); 
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void renderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(p);
	
	int loc = glGetUniformLocation(p, "angle"); //set rotation angle
	glUniform1f(loc, angle);

	//ToDo nakreslite trojuholnik
	glBindVertexArray (vao);
	// draw points 0-3 from the currently bound VAO with current in-use shader
	glDrawArrays (GL_TRIANGLES, 0, 3);

	angle += 0.1f;

    glutSwapBuffers();
}

int main(int argc, char **argv) {

	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(320,320);
	glutCreateWindow("OpenGL");

	// init GLEW to support extensions
	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
        printf("Ready for OpenGL 3.3\n");
    else {
        printf("OpenGL 3.3 not supported\n");
        exit(1);
    }

	p = createShaderProgram("./shaders/shader.vert","./shaders/shader.frag");

	glUseProgram(p);


	//This is the location of the L_pos uniform variable in our program
	int loc = glGetUniformLocation(p, "L_pos");
	glUniform3f(loc, 3.0, 0.0, 1.0);

	//This is the location of the L_pos uniform variable in our program
	loc = glGetUniformLocation(p, "diffuse_col");
	glUniform4f(loc, 1.0, 1.0, 0.0, 1.0);

	
	// set matrix with an array
	float m1[] = {cos(0.1*PI/180.0), -sin(0.1*PI/180.0), 0, 0, 
				  sin(0.1*PI/180.0),  cos(0.1*PI/180.0), 0, 0, 
					0, 0, 1, 0, 
					0, 0, 0, 1};
	float m2[] = {cos(0.1*PI/180.0), -sin(0.1*PI/180.0), 0, 
				  sin(0.1*PI/180.0), cos(0.1*PI/180.0), 0,
				  0, 0, 1};

	//This is the location of the rotation4 uniform variable in our program
	loc = glGetUniformLocation(p, "rotation4");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  m1);

	//This is the location of the rotation3 uniform variable in our program
	loc = glGetUniformLocation(p, "rotation3");
	glUniformMatrix3fv(loc, 1, GL_FALSE,  m2);

	setVertexArray();

	// register callbacks
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);

	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;
}