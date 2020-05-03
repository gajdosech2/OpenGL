#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_DEPRECATE

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
bool occlusion_enabled = true;

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

void loadScene(const std::string pFile)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Fast);

	if(!scene)
	{
		printf("%s\n", importer.GetErrorString());
		return;
	}

	aiMesh* tmpMesh = scene->mMeshes[0];
		
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

	glGenBuffers(1, &gTrianglesIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gTrianglesIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * faceCnt* 3, indexArr, GL_STATIC_DRAW);

	if (tmpMesh->HasPositions())
	{
		vPosArr = (float *)malloc(sizeof(float) * tmpMesh->mNumVertices * 3);
		index = 0;
		for (unsigned int t = 0; t < tmpMesh->mNumVertices; ++t)
		{
			memcpy(&vPosArr[index], &tmpMesh->mVertices[t], 3 * sizeof(float));
			index += 3;
		}
		vertCnt = tmpMesh->mNumVertices;

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
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, texture_width, texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
  
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);
 
	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if( status != GL_FRAMEBUFFER_COMPLETE )
		printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n");

	glDrawBuffer(GL_NONE);
 
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
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(p);

	int pass_location = glGetUniformLocation(p, "pass_number");
	glUniform1i(pass_location, 1);

	glBindVertexArray (vao);
	glDrawElements(GL_TRIANGLES, faceCnt* 3, GL_UNSIGNED_INT, NULL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glEnable(GL_TEXTURE_2D);
	glUniform1i(pass_location, 2);
	glDrawElements(GL_TRIANGLES, faceCnt* 3, GL_UNSIGNED_INT, NULL);
	
/*	
	///glColor3f(1,0,0);

	glBegin( GL_QUADS );
	glTexCoord2f( 0.0f, 0.0f ); 
	glVertex3f( -1.0f, -1.0f, 0.0f );
	glTexCoord2f( 1.0f, 0.0f ); 
	glVertex3f( 1.0f, -1.0f, 0.0f );
	glTexCoord2f( 1.0f, 1.0f ); 
	glVertex3f( 1.0f, 1.0f, 0.0f );	
	glTexCoord2f( 0.0f, 1.0f ); 
	glVertex3f( -1.0f, 1.0f, 0.0f );	
	glEnd( );

*/
   glutSwapBuffers();
}

void keyPressed(unsigned char key, int x, int y) 
{
	glUseProgram(p);
	int location = glGetUniformLocation(p, "enabled");
	if (key == 'a') 
	{
		occlusion_enabled = !occlusion_enabled;
		glUniform1i(location, occlusion_enabled);
	}
}

int main(int argc, char **argv) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("OpenGL");

	glewInit();
	glEnable(GL_DEPTH_TEST);
	if (glewIsSupported("GL_VERSION_3_3"))
        printf("Ready for OpenGL 3.3\n");
    else {
        printf("OpenGL 3.3 not supported\n");
        exit(1);
    }

	p = createShaderProgram1((char *)"./shaders/shader.vert", (char*)"./shaders/shader.frag");
	glUseProgram(p);
	
	loadScene("drak.dae");

	int loc = glGetUniformLocation(p, "L_pos");
	glUniform3f(loc, 10.0, 5.0, 10.0);

	loc = glGetUniformLocation(p, "diffuse_col");
	glUniform4f(loc, 0.7, 0.8, 1.0, 1.0);

	loc = glGetUniformLocation(p, "enabled");
	glUniform1i(loc, occlusion_enabled);

	Matrix4x4 model_view;
	model_view.Translate(1,0,-20);
	model_view.Rotate(45, 0.2, 0.1, 1);
	model_view.Rotate(60, 1, 1, 0);
	
	float * normal_matrix = new float[9];
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			normal_matrix[j + 3*i] = model_view.matrix[j + 4*i];

	Matrix4x4 projection;
	projection.setPerspective(30, 1.0f, 1.0f, 100.0f);

	loc = glGetUniformLocation(p, "camera_model_view_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  model_view.matrix);

	loc = glGetUniformLocation(p, "normal_matrix");
	glUniformMatrix3fv(loc, 1, GL_FALSE,  normal_matrix);

	loc = glGetUniformLocation(p, "camera_projection_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);

	loc = glGetUniformLocationARB(p, "depth_tex");
	glUniform1iARB(loc, 0);

	setVertexArray();
	setFBO(800, 800);
	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(keyPressed);

	glutMainLoop();
	return 1;
}