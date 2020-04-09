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
#include "tools/loadBMP.cpp"

#pragma comment(lib, "assimp.lib")

#include "Assimp/assimp.hpp"	
#include "Assimp/aiPostProcess.h"
#include "Assimp/aiScene.h"



/*
modelview matica 4x4
*/
// set matrix with an array
Matrix4x4 model_view;

//normal matrix 3x3
float * normal_matrix = new float[9];

/*
budeme mat 2 shader programy
*/
GLuint p1, p2, p3;  //shader programy

/*
budeme mat 3 objekty
*/
GLuint vao[3]; //Vertex Array Object

GLuint fb, depth_tex, color_tex[2]; //budeme mat 2 uv textury

bool occlusion_enabled = true;

float uhol = 0.0;

void updateParticles();//funkcie su implementovane nizsie
void renderParticles();

float t = 0;//pouzijeme pri animacii kridel
float L_inten;//intenzita svetla

/*
budeme mat 3 objekty, tak si data pre kazdy odjekt nacitame do prislusnej struktury pomocou loadScene(&od)
*/
struct ObjectData
{
	float *points;
	float *normals;
	unsigned int* indexArr;
	unsigned int index;
	unsigned int vertCnt;
	unsigned int faceCnt;
	GLuint gTrianglesIB;
	GLuint gPositionsVB;
	GLuint gNormalsVB;
	GLuint gTexCoordsVB;
	float* vPosArr;
};

//budeme mat 2 objekty, pricom druhy je rozdeleny na 2, teda celkovo 3
ObjectData objekt[3];

void loadScene(ObjectData &od, std::string name)
{
		std::string pFile = name; 
 
		// Create an instance of the Importer class
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile( pFile, aiProcessPreset_TargetRealtime_Fast);

			// If the import failed, report it
		if(!scene)
		{
			printf("%s\n", importer.GetErrorString());
			return;
		}

		// generate VBOs
		aiMesh* tmpMesh = scene->mMeshes[0];
		
		// triangles array
		od.indexArr = (unsigned int *)malloc(sizeof(unsigned int) * tmpMesh->mNumFaces * 3);
		od.index = 0;
		od.vertCnt = 0;
		od.faceCnt = 0;

		for (unsigned int t = 0; t < tmpMesh->mNumFaces; ++t)
		{
			const struct aiFace* face = &tmpMesh->mFaces[t];

			memcpy(&od.indexArr[od.index], face->mIndices,3 * sizeof(unsigned int));
			od.index += 3;
		}
		od.faceCnt = tmpMesh->mNumFaces;

		// buffer for faces
		glGenBuffers(1, &od.gTrianglesIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, od.gTrianglesIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * od.faceCnt* 3, od.indexArr, GL_STATIC_DRAW);

		// buffer for vertex positions
		if (tmpMesh->HasPositions())
		{
			// save position arrays for animation
			od.vPosArr = (float *)malloc(sizeof(float) * tmpMesh->mNumVertices * 3);
			od.index = 0;
			for (unsigned int t = 0; t < tmpMesh->mNumVertices; ++t)
			{
				memcpy(&od.vPosArr[od.index], &tmpMesh->mVertices[t], 3 * sizeof(float));
				od.index += 3;
			}
			od.vertCnt = tmpMesh->mNumVertices;

			glGenBuffers(1, &od.gPositionsVB);
			glBindBuffer(GL_ARRAY_BUFFER, od.gPositionsVB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*od.vertCnt, od.vPosArr, GL_STATIC_DRAW);
		}

		if (tmpMesh->HasNormals())
		{
			glGenBuffers(1, &od.gNormalsVB);
			glBindBuffer(GL_ARRAY_BUFFER, od.gNormalsVB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*od.vertCnt, tmpMesh->mNormals, GL_STATIC_DRAW);
		}

		if (tmpMesh->HasTextureCoords(0))
		{
			glGenBuffers(1, &od.gTexCoordsVB);
			glBindBuffer(GL_ARRAY_BUFFER, od.gTexCoordsVB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*od.vertCnt, tmpMesh->mTextureCoords[0], GL_STATIC_DRAW);
		}
}

void setFBO(int texture_width,int texture_height){

	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, texture_width, texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
  
	//Attach
	glFramebufferTexture(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, depth_tex, 0);
 
	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if( status != GL_FRAMEBUFFER_COMPLETE )
		printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n");

	glDrawBuffer(GL_NONE); // No color buffer is drawn to.
 
	// switch back to window-system-provided framebuffer
	// and unbind the texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	
}

void setVertexArray(){
	//Vertex Array Object
	glGenVertexArrays (3, vao);// vygenerujeme 2 vertex array objekty do pola vao

	glBindVertexArray (vao[0]); //prvy objekt
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[0].gPositionsVB);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[0].gNormalsVB);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[0].gTexCoordsVB);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objekt[0].gTrianglesIB);

	glBindVertexArray (vao[1]); //druhy objekt
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[1].gPositionsVB);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[1].gNormalsVB);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[1].gTexCoordsVB);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objekt[1].gTrianglesIB);

	glBindVertexArray (vao[2]); //treti objekt
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[2].gPositionsVB);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[2].gNormalsVB);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, objekt[2].gTexCoordsVB);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objekt[2].gTrianglesIB);

	glBindVertexArray (0); 
}

void renderScene(void) {

	glDisable (GL_BLEND); //nebudeme riesit alfa blending v tomto prechode
	glEnable(GL_DEPTH_TEST);//budeme riesit viditelnost

	/*
	kreslime prvy model s p1 shaderom
	*/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);//v prvom prechode budeme kreslit do depth_tex

	glClear(GL_DEPTH_BUFFER_BIT);//vycistime depth buffer

	glUseProgram(p1);

	//resetuj modelview maticu
	model_view.LoadIdentity();

	//posunieme objekt od kamery o 30 a zrotujeme
	model_view.Translate(1,0,-30);
	model_view.Rotate(45, 0.2, 0.1, 1);
	model_view.Rotate(60, 1, 1, 0);

	//kedze nepouzivame neuniformne skalovanie
	//skopirujeme cast modelovacej matice do normalovej
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			normal_matrix[j + 3*i] = model_view.matrix[j + 4*i];

	/*
	posleme modelview a normalovu maticu shader programu p1
	*/
	//This is the location of the camera view matrix uniform variable in our program
	int loc = glGetUniformLocation(p1, "camera_model_view_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  model_view.matrix);

	//This is the location of the normal matrix uniform variable in our program
	loc = glGetUniformLocation(p1, "normal_matrix");
	glUniformMatrix3fv(loc, 1, GL_FALSE,  normal_matrix);

	L_inten = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 0.3f + 0.9f;
	loc = glGetUniformLocation(p1, "L_inten");
	glUniform1f(loc, L_inten);

	int pass_location = glGetUniformLocation(p1, "pass_number");
	glUniform1i(pass_location, 1);

	//renderujeme prvy objekt
	glBindVertexArray (vao[0]);
	// draw triangles from the currently bound VAO with current in-use shader
	glDrawElements(GL_TRIANGLES, objekt[0].faceCnt* 3, GL_UNSIGNED_INT, NULL);

	//v dalsom prechode prestaneme kreslit do depth_tex
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//vycistime frame a Z-buffer

	/*
	pouzijeme multitexturovanie,
	konkretne texturnu jednotku 0 s depth texturou
	a texturnu jednotku 1 s color_tex
	*/
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, depth_tex);

	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, color_tex[0]);

	glEnable(GL_TEXTURE_2D);

	glUniform1i(pass_location, 2);

	// draw triangles from the currently bound VAO with current in-use shader
	glDrawElements(GL_TRIANGLES, objekt[0].faceCnt* 3, GL_UNSIGNED_INT, NULL);
	
	/*
	ideme renderovat druhy objekt
	*/
	glUseProgram(p2);//zapneme druhy program

	/*
	zmenime modelview a normalovu maticu pre druhy objekt
	*/
	//resetuj modelview maticu
	model_view.LoadIdentity();

	//posunieme objekt od kamery o 30 a zrotujeme
	model_view.Translate(1, 0, -30);
	model_view.Rotate(90, 1, 0, 0);
	model_view.Rotate(uhol, 0, 0, -1);
	
	model_view.Translate(10, 0, 10);
	model_view.Rotate(-90, 0,0, 1);
	
	//kedze nepouzivame neuniformne skalovanie
	//skopirujeme cast modelovacej matice do normalovej
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			normal_matrix[j + 3*i] = model_view.matrix[j + 4*i];

	/*
	posleme modelview a normalovu maticu shader programu p2
	*/
	//This is the location of the camera view matrix uniform variable in our program
	loc = glGetUniformLocation(p2, "camera_model_view_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  model_view.matrix);

	//This is the location of the normal matrix uniform variable in our program
	loc = glGetUniformLocation(p2, "normal_matrix");
	glUniformMatrix3fv(loc, 1, GL_FALSE,  normal_matrix);

	glActiveTexture(GL_TEXTURE0); //texturna jednotka 0
	glBindTexture(GL_TEXTURE_2D, color_tex[1]);//bude teraz obsahovat druhu uv mapu

	loc = glGetUniformLocation(p2, "L_inten");
	glUniform1f(loc, L_inten);
	
	loc = glGetUniformLocation(p2, "_t");
	glUniform1f(loc, t);

	loc = glGetUniformLocation(p2, "anim");
	glUniform1i(loc, false);

	//renderujeme druhy objekt
	glBindVertexArray (vao[1]);
	// draw triangles from the currently bound VAO with current in-use shader
	glDrawElements(GL_TRIANGLES, objekt[1].faceCnt* 3, GL_UNSIGNED_INT, NULL);

	glUniform1i(loc, true);

	//renderujeme treti objekt
	glBindVertexArray (vao[2]);
	// draw triangles from the currently bound VAO with current in-use shader
	glDrawElements(GL_TRIANGLES, objekt[2].faceCnt* 3, GL_UNSIGNED_INT, NULL);

	glBindVertexArray (0);

	/*
	ideme renderovat castice
	*/
	glEnable (GL_BLEND); //budeme pouzivat alfa blending
	glDisable(GL_DEPTH_TEST);//nebudeme pouzivat z-buffer

	glUseProgram(p3);//zapneme treti program

	//resetuj modelview maticu
	model_view.LoadIdentity();

	//posunieme objekt od kamery o 30 a zrotujeme
	model_view.Translate(1, 0, -30);
	model_view.Rotate(90, 1, 0, 0);
	model_view.Rotate(45, 0, 0, 1);
	model_view.Translate(0, -10, 1);

	loc = glGetUniformLocation(p3, "camera_model_view_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE,  model_view.matrix);

	renderParticles();

    glutSwapBuffers();
}

void keyPressed (unsigned char key, int x, int y) {  

	glUseProgram(p1);//nastavujeme uniformu v programe p1, preto ho musime zapnut

	int location = glGetUniformLocation(p1, "enabled");

	if  (key == 'a') {	// If they ‘a’ key was pressed  
						// Perform action associated with the ‘a’ key  

		if (occlusion_enabled) {

			occlusion_enabled = false;
					
		} else {

			occlusion_enabled = true;
		
		}

		glUniform1i(location, occlusion_enabled);

	} 

}

// CPU representation of a particle
struct Particle{
	float x, y, z;//pozicia
	float dx, dy, dz;//smer
	float size;
	int life; // Remaining life of the particle. if < 0 : dead and unused.
};

// The VBO containing the 4 vertices of the particles.
// Thanks to instancing, they will be shared by all particles.
static const GLfloat g_vertex_buffer_data[] = {
 0, 0, 0.0f,
 1, 0, 0.0f,
 0, 1, 0.0f,
 1, 1, 0.0f,
};

Particle * particles;

GLfloat * g_particule_position_size_data;

GLuint billboard_vertex_buffer;
GLuint particles_position_size_buffer;

int ParticlesCount; 

void setParticles(int count) {
	ParticlesCount = count;

	g_particule_position_size_data = new float[ParticlesCount * 4];

	particles = new Particle[ParticlesCount];

	for (int i = 0; i < ParticlesCount; i++)
	{
		

		//Fill the structure
			particles[i].x = 0; particles[i].y = 0; particles[i].z = 0;
			particles[i].dx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f)/20.0f; 
			particles[i].dy = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 2)/10.0f; 
			particles[i].dz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f)/20.0f; 
			particles[i].size = 0.5f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX); 
			particles[i].life = 30 + (rand() % static_cast<int>(30 - 10 + 1));//nahodne cislo od 10 po 30

		// Fill the GPU buffer
            g_particule_position_size_data[4*i+0] = particles[i].x;
            g_particule_position_size_data[4*i+1] = particles[i].y;
            g_particule_position_size_data[4*i+2] = particles[i].z;
			
            g_particule_position_size_data[4*i+3] = particles[i].size;
	}

	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particles_position_size_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_size_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, ParticlesCount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_particule_position_size_data), g_particule_position_size_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void updateParticles(){
	// Update the buffers that OpenGL uses for rendering.
	// There are much more sophisticated means to stream data from the CPU to the GPU,
	// but this is outside the scope of this tutorial.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming

	for (int i = 0; i < ParticlesCount; i++)
	{
		particles[i].x += particles[i].dx; 
		particles[i].y += particles[i].dy; 
		particles[i].z += particles[i].dz; 

		particles[i].size *= 1.03f;//postupne zvascujeme casticu

		
		if (--particles[i].life < 0){
			particles[i].x = 0; particles[i].y = 0; particles[i].z = 0;
			particles[i].dx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f)/20.0f; 
			particles[i].dy = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 2)/10.0f; 
			particles[i].dz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f)/20.0f; 
			particles[i].size = 0.5f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX); 
			particles[i].life = 50 + (rand() % static_cast<int>(50 - 30 + 1));//nahodne cislo od 30 po 50
		}

		// Fill the GPU buffer
        g_particule_position_size_data[4*i+0] = particles[i].x;
        g_particule_position_size_data[4*i+1] = particles[i].y;
        g_particule_position_size_data[4*i+2] = particles[i].z;
        g_particule_position_size_data[4*i+3] = particles[i].size;
	}

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_size_buffer);
	glBufferData(GL_ARRAY_BUFFER, ParticlesCount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);
}

void renderParticles(){
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
	 0, // attribute. No particular reason for 0, but must match the layout in the shader.
	 3, // size
	 GL_FLOAT, // type
	 GL_FALSE, // normalized?
	 0, // stride
	 (void*)0 // array buffer offset
	);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_size_buffer);
	glVertexAttribPointer(
	 1, // attribute. No particular reason for 1, but must match the layout in the shader.
	 4, // size : x + y + z + size
	 GL_FLOAT, // type
	 GL_FALSE, // normalized?
	 0, // stride
	 (void*)0 // array buffer offset
	);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1

	// Draw the particules !
	// This draws many times a small triangle_strip (which looks like a quad).
	// This is equivalent to :
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
}

static void timerCallback (int value) { 
	/* Do timer processing */ 
	t += 0.1;
	if (t >= M_PI) t -= M_PI;
	//printf("%f\n",t);
	uhol += 1.5;
	if (uhol > 360.0) uhol = 0.0;
	updateParticles();
	glutPostRedisplay();
	/* call back again after 20 milliseconds have passed */ 
	glutTimerFunc (20, timerCallback, value); 
}

int main(int argc, char **argv) {

	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(512,512);
	glutCreateWindow("OpenGL");

	// init GLEW to support extensions
	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
        printf("Ready for OpenGL 3.3\n");
    else {
        printf("OpenGL 3.3 not supported\n");
        exit(1);
    }

	p1 = createShaderProgram((char*)"./shaders/ambient.vert", (char*)"./shaders/ambient.frag");// ambient oclusion program
	p2 = createShaderProgram((char*)"./shaders/vtak.vert", (char*)"./shaders/vtak.frag");// vtak a jeho animacia v shader programe
	p3 = createShaderProgram((char*)"./shaders/particles.vert", (char*)"./shaders/particles.frag");// particles
	
	/*
	najskor nastavime program p1 pre prvy model
	*/
	glUseProgram(p1);
	
	loadScene(objekt[0], "drak.dae");

	//This is the location of the L_pos uniform variable in our program
	int loc = glGetUniformLocation(p1, "L_pos");
	glUniform3f(loc, -10.0, 2.0, -10.0);

	//This is the location of the L_inten uniform variable in our program
	loc = glGetUniformLocation(p1, "L_inten");
	glUniform1f(loc, 1.0f);

	//This is the location of the enabled uniform variable in our program
	loc = glGetUniformLocation(p1, "enabled");
	glUniform1i(loc, occlusion_enabled);


	//0 is the texture unit where we want to bind the depth texture
	loc = glGetUniformLocationARB(p1, "depth_tex");
	glUniform1iARB(loc, 0);

	//1 is the texture unit where we want to bind my uv texture
	loc = glGetUniformLocationARB(p1, "color_tex");
	glUniform1iARB(loc, 1);
	
	/*
	teraz nastavime program p2 a druhy model
	*/
	glUseProgram(p2);
	

	loadScene(objekt[1], "bird_torso.dae");
	loadScene(objekt[2], "bird_wings.dae");


	//This is the location of the L_pos uniform variable in our program
	loc = glGetUniformLocation(p2, "L_pos");
	glUniform3f(loc, -15.0, 1.0, -5.0);

	//0 is the texture unit where we want to bind the depth texture
	loc = glGetUniformLocationARB(p1, "uv_tex");
	glUniform1iARB(loc, 0);


	/*
	projekcna matica (vnutorne parametre kamery) je pre oba shader programy rovnaka a pocas behu sa nemeni
	*/
	Matrix4x4 projection;

	projection.setPerspective(30, 1.0f, 1.0f, 100.0f);

	glUseProgram(p1);
	//This is the location of the camera projection matrix uniform variable in our program
	loc = glGetUniformLocation(p1, "camera_projection_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);

	
	glUseProgram(p2);
	//This is the location of the camera projection matrix uniform variable in our program
	loc = glGetUniformLocation(p2, "camera_projection_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);

	glUseProgram(p3);
	//This is the location of the camera projection matrix uniform variable in our program
	loc = glGetUniformLocation(p3, "camera_projection_matrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);

/*
load texture
*/
	// Actual RGB data
	unsigned char * data = loadBMP("UVtextura_drak.bmp");
	// Create one OpenGL texture
	glGenTextures(2, color_tex);//vytvorime 2 textury
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, color_tex[0]);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//nacitame drhu texturu
	// Actual RGB data
	data = loadBMP("UVtextura_vtak.bmp");
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, color_tex[1]);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	setVertexArray();

	setFBO(512, 512);

	setParticles(100);

	//budeme pouzivat alfa blending tak tu nastavime sposob miesania
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;

	// register callbacks
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
	glutTimerFunc(10, timerCallback, 0); //be triggered in a specified number of milliseconds

	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;
}