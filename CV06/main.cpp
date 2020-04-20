#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include "GL/glew.c"
#include "GL/glut.h"
#include "tools/glslang.cpp"
#include "tools/matrix4x4.cpp"

#pragma comment(lib, "assimp.lib")

#include "Assimp/assimp.hpp"	
#include "Assimp/aiPostProcess.h"
#include "Assimp/aiScene.h"


GLuint p;
GLuint vao;
GLuint fb, depth_tex; 

float *points = 0;
float *normals = 0;
unsigned int* indexArr = 0;
unsigned int index = 0;
unsigned int vertCnt = 0;
unsigned int faceCnt = 0;
GLuint gTrianglesIB = 0;
GLuint gPositionsVB = 0;
GLuint gNormalsVB = 0;
float* vPosArr = 0;

void loadScene(std::string name)
{
	std::string pFile = name; 
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile( pFile, aiProcessPreset_TargetRealtime_Fast);

	if(!scene)
	{
		printf("%s\n", importer.GetErrorString());
		return;
	}

	// generate VBOs
	aiMesh* tmpMesh = scene->mMeshes[0];
		
	// triangles array
	indexArr = (unsigned int *)malloc(sizeof(unsigned int) * tmpMesh->mNumFaces * 3);
	index = 0;
	vertCnt = 0;
	faceCnt = 0;

	for (unsigned int t = 0; t < tmpMesh->mNumFaces; ++t)
	{
		const struct aiFace* face = &tmpMesh->mFaces[t];

		memcpy(&indexArr[index], face->mIndices,3 * sizeof(unsigned int));
		index += 3;
	}
	faceCnt = tmpMesh->mNumFaces;

	// buffer for faces
	glGenBuffers(1, &gTrianglesIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gTrianglesIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * faceCnt* 3, indexArr, GL_STATIC_DRAW);
		
	if (tmpMesh->HasPositions())
	{
		// save position arrays for animation
		vPosArr = (float *)malloc(sizeof(float) * tmpMesh->mNumVertices * 3);
		index = 0;
		for (unsigned int t = 0; t < tmpMesh->mNumVertices; ++t)
		{
			memcpy(&vPosArr[index], &tmpMesh->mVertices[t], 3 * sizeof(float));
			index += 3;
		}
		vertCnt = tmpMesh->mNumVertices;

		// buffer for vertex positions
		glGenBuffers(1, &gPositionsVB);
		glBindBuffer(GL_ARRAY_BUFFER, gPositionsVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*vertCnt, vPosArr, GL_STATIC_DRAW);
	}

	if (tmpMesh->HasNormals())
	{
		glGenBuffers(1, &gNormalsVB);
		glBindBuffer(GL_ARRAY_BUFFER, gNormalsVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*vertCnt, tmpMesh->mNormals, GL_STATIC_DRAW);
	}
}

void setFBO(int texture_width,int texture_height)
{
	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexImage2D (...);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
  
	//Attach depth_tex
	//glFramebufferTexture(...);

	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if( status != GL_FRAMEBUFFER_COMPLETE )
		printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n");

	glDrawBuffer(GL_NONE); // No color buffer is drawn to.
 
	// switch back to window-system-provided framebuffer
	// and unbind the texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setVertexArray()
{
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, gPositionsVB);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, gNormalsVB);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gTrianglesIB);
}

void renderScene(void) 
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(p);

	int pass_location = glGetUniformLocation(p, "pass_number");
	glUniform1i(pass_location, 1);

	glBindVertexArray (vao);
	glDrawElements(GL_TRIANGLES, faceCnt* 3, GL_UNSIGNED_INT, NULL);

	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, depth_tex);

	glEnable(GL_TEXTURE_2D);

	glUniform1i(pass_location, 2);

	glDrawElements(GL_TRIANGLES, faceCnt* 3, GL_UNSIGNED_INT, NULL);

	glutSwapBuffers();
}

int main(int argc, char **argv) 
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(512,512);
	glutCreateWindow("OpenGL");

	// init GLEW to support extensions
	glewInit();
	glEnable(GL_DEPTH_TEST);
	if (glewIsSupported("GL_VERSION_3_3"))
        printf("Ready for OpenGL 3.3\n");
    else {
        printf("OpenGL 3.3 not supported\n");
        exit(1);
    }

	p = createShaderProgram((char *)"./shaders/shader.vert", (char*)"./shaders/shader.geom", (char*)"./shaders/shader.frag");
	glUseProgram(p);
	loadScene("chobotnicka2.dae");

	//This is the location of the L_pos uniform variable in our program
	int loc = glGetUniformLocation(p, "L_pos");
	glUniform3f(loc, 0.0, 0.0, 10.0);

	//This is the location of the L_pos uniform variable in our program
	loc = glGetUniformLocation(p, "diffuse_col");
	glUniform4f(loc, 1.0, 1.0, 0.0, 1.0);

	// set matrix with an array
	Matrix4x4 model_view;

	//ToDo nastavte vhodny pohlad z kamery
	//model_view.Translate(...);
	//model_view....

	float * normal_matrix = new float[9];
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			normal_matrix[j + 3 * i] = model_view.matrix[j + 4 * i];

	Matrix4x4 projection;
	projection.setPerspective(30, 1.0f, 0.1f, 100.0f);

	Matrix4x4 light_modelview;
	light_modelview.Translate(0, 0, -2);
	light_modelview.Scale(0.1, 0.1, 0.1);
	light_modelview.Rotate(20, 0, 1, 0);

	Matrix4x4 light_projection;

	//ToDo nastavte kameru z pozicie svetla
	//light_projection.setOrtho(...);

	//ToDo nastavte uniformy
	//This is the location of the camera view matrix uniform variable in our program
	loc = glGetUniformLocation(p, "camera_model_view_matrix");
	//glUniformMatrix4fv(...);

	//This is the location of the normal matrix uniform variable in our program
	loc = glGetUniformLocation(p, "normal_matrix");
	//glUniformMatrix3fv(...);

	//This is the location of the camera projection matrix uniform variable in our program
	loc = glGetUniformLocation(p, "camera_projection_matrix");
	//glUniformMatrix4fv(...);

	//This is the location of the view from the light position matrix uniform variable in our program
	loc = glGetUniformLocation(p, "light_model_view_matrix");
	//glUniformMatrix4fv(...);

	//This is the location of the view from the light projection uniform variable in our program
	loc = glGetUniformLocation(p, "light_projection_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, light_projection.matrix);

	//0 is the texture unit where I want to bind my texture
	loc = glGetUniformLocationARB(p, "depth_tex");
	glUniform1iARB(loc, 0);

	setVertexArray();

	setFBO(512, 512);

	glEnable(GL_DEPTH_TEST);

	// register callbacks
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);

	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;
}