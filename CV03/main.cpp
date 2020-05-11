#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include "GL/glew.c"
#include "GL/glut.h"
#include "tools/glslang.cpp"
#include "tools/matrix4x4.cpp"
#include "tools/loadBMP.cpp"

#pragma comment(lib, "assimp.lib")

#include "Assimp/assimp.hpp"	
#include "Assimp/aiPostProcess.h"
#include "Assimp/aiScene.h"

GLuint shader_program;
GLuint color_tex;

Matrix4x4 model_view, projection;
float* normal_matrix = new float[9];

struct ObjectBuffer
{
	GLuint gTrianglesIB;
	GLuint gPositionsVB;
	GLuint gNormalsVB;
	GLuint gTexCoordsVB;
	GLuint gVertexAO;
	unsigned int faceCnt;
};

ObjectBuffer ob[1];
float* vPosArr = 0;

void loadScene(ObjectBuffer &o, std::string name)
{
	{
		std::string pFile = name;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Fast);

		if (!scene)
		{
			printf("%s\n", importer.GetErrorString());
			return;
		}

		aiMesh* tmpMesh = scene->mMeshes[0];

		unsigned int* indexArr = (unsigned int*)malloc(sizeof(unsigned int) * tmpMesh->mNumFaces * 3);
		int index = 0;
		int vertCnt = 0;
		const struct aiFace* face;
		for (unsigned int t = 0; t < tmpMesh->mNumFaces; ++t)
		{
			face = &tmpMesh->mFaces[t];
			memcpy(&indexArr[index], face->mIndices, 3 * sizeof(unsigned int));
			index += 3;
		}
		o.faceCnt = tmpMesh->mNumFaces;

		glGenBuffers(1, &o.gTrianglesIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o.gTrianglesIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * o.faceCnt * 3, indexArr, GL_STATIC_DRAW);

		free(indexArr);

		if (tmpMesh->HasPositions())
		{
			vPosArr = (float*)malloc(sizeof(float) * tmpMesh->mNumVertices * 3);
			index = 0;
			for (unsigned int t = 0; t < tmpMesh->mNumVertices; ++t)
			{
				memcpy(&vPosArr[index], &tmpMesh->mVertices[t], 3 * sizeof(float));
				index += 3;
			}
			vertCnt = tmpMesh->mNumVertices;

			glGenBuffers(1, &o.gPositionsVB);
			glBindBuffer(GL_ARRAY_BUFFER, o.gPositionsVB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * vertCnt, vPosArr, GL_STATIC_DRAW);

			free(vPosArr);
		}

		if (tmpMesh->HasNormals())
		{
			glGenBuffers(1, &o.gNormalsVB);
			glBindBuffer(GL_ARRAY_BUFFER, o.gNormalsVB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * vertCnt, tmpMesh->mNormals, GL_STATIC_DRAW);
		}

		float* texCoordArr = 0;
		float uv[2];
		if (tmpMesh->HasTextureCoords(0))
		{
			texCoordArr = (float*)malloc(sizeof(float) * vertCnt * 2);
			index = 0;
			for (unsigned int t = 0; t < vertCnt; ++t)
			{
				uv[0] = tmpMesh->mTextureCoords[0][t].x;
				uv[1] = tmpMesh->mTextureCoords[0][t].y;
				memcpy(&texCoordArr[index], &uv, 2 * sizeof(float));
				index += 2;
			}
			glGenBuffers(1, &o.gTexCoordsVB);
			glBindBuffer(GL_ARRAY_BUFFER, o.gTexCoordsVB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * vertCnt, texCoordArr, GL_STATIC_DRAW);

			free(texCoordArr);
		}
	}

	// Assignment 1: Create VAO
	glGenVertexArrays (1, &o.gVertexAO);
	glBindVertexArray (o.gVertexAO); 

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, o.gPositionsVB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, o.gNormalsVB);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, o.gTexCoordsVB);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o.gTrianglesIB);

	glBindVertexArray(0);
}

void renderScene(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program);
	
	int loc = glGetUniformLocation(shader_program, "camera_projection_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);
	
	//kedze nepouzivame neuniformne skalovanie skopirujeme cast modelovacej matice do normalovej
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			normal_matrix[j + 3*i] = model_view.matrix[j + 4*i];

	loc = glGetUniformLocation(shader_program, "camera_model_view_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  model_view.matrix);

	loc = glGetUniformLocation(shader_program, "normal_matrix");
	glUniformMatrix3fv(loc, 1, GL_FALSE,  normal_matrix);

	glBindVertexArray (ob[0].gVertexAO);

	glDrawElements(GL_TRIANGLES, ob[0].faceCnt* 3, GL_UNSIGNED_INT, NULL);

	glutSwapBuffers();
}

void reshape(int w,int h)
{ 
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	projection.LoadIdentity();
	projection.setPerspective(
						/* field of view in degree */40.0, 
						/* aspect ratio */(GLfloat) w/(GLfloat) h, 
						/* Z near */0.1, 
						/* Z far */100.0); 
	model_view.LoadIdentity();
	model_view.Translate(5.0, 1.0, -15.0);
	model_view.Rotate(45.0, 0.0, 1.0, 0.0);
	model_view.Scale(1.5, 1.5, 1.5);
}

int main(int argc, char** argv)
{
	{
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(800, 600);
		glutCreateWindow("OpenGL - Exercise 03 Ant");

		glewInit();
		if (!glewIsSupported("GL_VERSION_3_3"))
			exit(1);

		shader_program = createShaderProgram("./shaders/shader.vert", "./shaders/shader.frag");
	}

	// Assignment 2: OpenGL Texture Part I
	glEnable(GL_TEXTURE_2D);

	unsigned char* data = loadBMP("uvmapa_original.bmp");

	glGenTextures(1, &color_tex);
	glBindTexture(GL_TEXTURE_2D, color_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	{
		loadScene(ob[0], "mravec.dae");

		glUseProgram(shader_program);

		int loc = glGetUniformLocation(shader_program, "L_pos");
		glUniform3f(loc, 10.0, 0.0, 1.0);

		loc = glGetUniformLocation(shader_program, "tex");
		glUniform1i(loc, 0);

		glutDisplayFunc(renderScene);
		glutReshapeFunc(reshape);
		glutIdleFunc(renderScene);
	
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
		
		glutMainLoop();
	}
	return 1;
}
