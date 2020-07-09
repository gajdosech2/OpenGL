#define _XKEYCHECK_H
#define GLFW_INCLUDE_NONE
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_DEPRECATE

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "Assimp/assimp.hpp"	
#include "Assimp/aiPostProcess.h"
#include "Assimp/aiScene.h"

#include "tools/glslang.cpp"
#include "tools/matrix4x4.cpp"
#include "tools/loadBMP.cpp"

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "assimp.lib")

struct ObjectData
{
    float* points;
    float* normals;
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

GLFWwindow* window;

GLuint programs[5];
ObjectData objects[4];
GLuint vaos[4];
GLuint color_tex[4];
GLuint fb, glow_tex, depth_rb;

Matrix4x4 model;
Matrix4x4 view;

float world_angle = 0;
float ufo_angle = 0;
float missile_angle = 0;
float missile_position = 22;
bool missile_active = false;
bool glow_enabled = true;
int rotation = 1;

float px = 0, py = 0, pz = 0;

void loadObject(ObjectData& od, const std::string file)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcessPreset_TargetRealtime_Fast);

    if (!scene)
    {
        printf("%s\n", importer.GetErrorString());
        return;
    }

    aiMesh* tmpMesh = scene->mMeshes[0];

    od.indexArr = (unsigned int*)malloc(sizeof(unsigned int) * tmpMesh->mNumFaces * 3);
    od.index = 0;
    od.vertCnt = 0;
    od.faceCnt = 0;

    for (unsigned int t = 0; t < tmpMesh->mNumFaces; ++t)
    {
        const struct aiFace* face = &tmpMesh->mFaces[t];

        memcpy(&od.indexArr[od.index], face->mIndices, 3 * sizeof(unsigned int));
        od.index += 3;
    }
    od.faceCnt = tmpMesh->mNumFaces;

    glGenBuffers(1, &od.gTrianglesIB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, od.gTrianglesIB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * od.faceCnt * 3, od.indexArr, GL_STATIC_DRAW);

    if (tmpMesh->HasPositions())
    {
        od.vPosArr = (float*)malloc(sizeof(float) * tmpMesh->mNumVertices * 3);
        od.index = 0;
        for (unsigned int t = 0; t < tmpMesh->mNumVertices; ++t)
        {
            memcpy(&od.vPosArr[od.index], &tmpMesh->mVertices[t], 3 * sizeof(float));
            od.index += 3;
        }
        od.vertCnt = tmpMesh->mNumVertices;

        glGenBuffers(1, &od.gPositionsVB);
        glBindBuffer(GL_ARRAY_BUFFER, od.gPositionsVB);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * od.vertCnt, od.vPosArr, GL_STATIC_DRAW);
    }

    if (tmpMesh->HasNormals())
    {
        glGenBuffers(1, &od.gNormalsVB);
        glBindBuffer(GL_ARRAY_BUFFER, od.gNormalsVB);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * od.vertCnt, tmpMesh->mNormals, GL_STATIC_DRAW);
    }

    if (tmpMesh->HasTextureCoords(0))
    {
        glGenBuffers(1, &od.gTexCoordsVB);
        glBindBuffer(GL_ARRAY_BUFFER, od.gTexCoordsVB);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * od.vertCnt, tmpMesh->mTextureCoords[0], GL_STATIC_DRAW);
    }
}

void setFBO(int texture_width, int texture_height)
{
    glGenTextures(1, &glow_tex);
    glBindTexture(GL_TEXTURE_2D, glow_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture_width, texture_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    // Attach 2D texture to this FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glow_tex, 0);

    glGenRenderbuffers(1, &depth_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, texture_width, texture_height);

    //Attach depth buffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
}

void setVertexArray()
{
    glGenVertexArrays(4, vaos);
    for (int i = 0; i < 4; i++)
    {
        glBindVertexArray(vaos[i]);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, objects[i].gPositionsVB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, objects[i].gNormalsVB);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, objects[i].gTexCoordsVB);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[i].gTrianglesIB);
    }
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        glUseProgram(programs[0]);
        int loc = glGetUniformLocation(programs[0], "enabled");
        glow_enabled = !glow_enabled;
        glUniform1i(loc, glow_enabled);
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        rotation = (rotation + 1) % 3;
        std::cout << rotation << std::endl;
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS && missile_active == false)
        missile_active = true;

    if (key == GLFW_KEY_A) px += 0.1f;

    if (key == GLFW_KEY_D) px -= 0.1f;

    if (key == GLFW_KEY_F) py += 0.1f;

    if (key == GLFW_KEY_R) py -= 0.1f;

    if (key == GLFW_KEY_W) pz += 0.1f;

    if (key == GLFW_KEY_S) pz -= 0.1f;
}


// CPU representation of a particle
struct Particle {
    float x, y, z;
    float dx, dy, dz;
    float size;
    int life; // Remaining life of the particle. if < 0 : dead and unused.
};

// The VBO containing the 4 vertices of the particles. Thanks to instancing, they will be shared by all particles.
static const GLfloat g_vertex_buffer_data[] = {
    0, 0, 0,
    1, 0, 0,
    0, 1, 0,
    1, 1, 0,
};

Particle* particles;

GLfloat* g_particule_position_size_data;

GLuint billboard_vertex_buffer;
GLuint particles_position_size_buffer;

int ParticlesCount = 100;

void setParticles() {
    g_particule_position_size_data = new float[ParticlesCount * 4];

    particles = new Particle[ParticlesCount];

    for (int i = 0; i < ParticlesCount; i++)
    {
        // Fill the structure
        particles[i].x = 0; particles[i].y = 0; particles[i].z = 0;
        particles[i].dx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) / 20.0f;
        particles[i].dy = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 2) / 10.0f;
        particles[i].dz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) / 20.0f;
        particles[i].size = 0.5f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        particles[i].life = 30 + (rand() % static_cast<int>(30 - 10 + 1)); //nahodne cislo od 10 po 30

        // Fill the GPU buffer
        g_particule_position_size_data[4 * i + 0] = particles[i].x;
        g_particule_position_size_data[4 * i + 1] = particles[i].y;
        g_particule_position_size_data[4 * i + 2] = particles[i].z;

        g_particule_position_size_data[4 * i + 3] = particles[i].size;
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

void updateParticles() {
    // Update the buffers that OpenGL uses for rendering.
    // There are much more sophisticated means to stream data from the CPU to the GPU,
    // but this is outside the scope of this tutorial.
    // http://www.opengl.org/wiki/Buffer_Object_Streaming

    for (int i = 0; i < ParticlesCount; i++)
    {
        particles[i].x += particles[i].dx;
        particles[i].y += particles[i].dy;
        particles[i].z += particles[i].dz;

        particles[i].size *= 1.03f; // postupne zvascujeme casticu


        if (--particles[i].life < 0) {
            particles[i].x = 0; particles[i].y = 0; particles[i].z = 0;
            particles[i].dx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) / 20.0f;
            particles[i].dy = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 2) / 10.0f;
            particles[i].dz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) / 20.0f;
            particles[i].size = 0.5f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            particles[i].life = 50 + (rand() % static_cast<int>(50 - 30 + 1)); // nahodne cislo od 30 po 50
        }

        // Fill the GPU buffer
        g_particule_position_size_data[4 * i + 0] = particles[i].x;
        g_particule_position_size_data[4 * i + 1] = particles[i].y;
        g_particule_position_size_data[4 * i + 2] = particles[i].z;
        g_particule_position_size_data[4 * i + 3] = particles[i].size;
    }

    glBindBuffer(GL_ARRAY_BUFFER, particles_position_size_buffer);
    glBufferData(GL_ARRAY_BUFFER, ParticlesCount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);
}

void renderParticles() 
{
    glBindVertexArray(0);

    glUseProgram(programs[3]);
    model.LoadIdentity();
    model.Translate(0, -13, 5);
    model.Scale(0.5f, 0.5f, 0.5f);
    model.Rotate(90, 1, 0, 0);

    int loc = glGetUniformLocation(programs[3], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

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


void renderBackground()
{
    glUseProgram(programs[4]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex[3]);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(-1, -1, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1, -1, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1, 1, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1, 1, 0);
    glEnd();
}

float* copyNormal()
{
    float* normal_matrix = new float[9];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            normal_matrix[j + 3 * i] = model.matrix[j + 4 * i];
    return normal_matrix;
}

void renderUFO()
{
    glUseProgram(programs[2]);

    model.LoadIdentity();
    model.Translate(0, -13, 3);
    model.Scale(1.5f, 1.5f, 1.5f);
    model.Rotate(ufo_angle, 0, 0, 1);
    model.Rotate(10, 0, 1, 0);

    ufo_angle = 0 ? (ufo_angle + 2 >= 360) : ufo_angle + 2;

    int loc = glGetUniformLocation(programs[2], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

    loc = glGetUniformLocation(programs[2], "normal_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, copyNormal());

    loc = glGetUniformLocation(programs[2], "L2_intensity");
    glUniform1f(loc, static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 0.5f + 0.5f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex[1]);
    glEnable(GL_TEXTURE_2D);

    glBindVertexArray(vaos[1]);
    glDrawElements(GL_TRIANGLES, objects[1].faceCnt * 3, GL_UNSIGNED_INT, NULL);
}

void renderFighters(int pass)
{
    glUseProgram(programs[0]);
    int pass_location = glGetUniformLocation(programs[0], "pass_number");
    glUniform1i(pass_location, pass);

    if (pass == 1)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        model.LoadIdentity();
        model.Translate(10, 10, -4);
        model.Rotate(-70, 0, 0, 1);
        model.Scale(0.3f, 0.3f, 0.3f);

        int loc = glGetUniformLocation(programs[0], "model_matrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

        loc = glGetUniformLocation(programs[0], "normal_matrix");
        glUniformMatrix3fv(loc, 1, GL_FALSE, copyNormal());

        glBindVertexArray(vaos[2]);
        glDrawElements(GL_TRIANGLES, objects[2].faceCnt * 3, GL_UNSIGNED_INT, NULL);

        model.LoadIdentity();
        model.Translate(-11, 4, 7);
        model.Rotate(-120, 0, 0, 1);
        model.Scale(0.3f, 0.3f, 0.3f);

        loc = glGetUniformLocation(programs[0], "model_matrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

        loc = glGetUniformLocation(programs[0], "normal_matrix");
        glUniformMatrix3fv(loc, 1, GL_FALSE, copyNormal());

        glBindVertexArray(vaos[2]);
        glDrawElements(GL_TRIANGLES, objects[2].faceCnt * 3, GL_UNSIGNED_INT, NULL);
    }

    if (pass == 2)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glow_tex);
        glEnable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex3f(-1, -1, 0);
            glTexCoord2f(1, 0);
            glVertex3f(1, -1, 0);
            glTexCoord2f(1, 1);
            glVertex3f(1, 1, 0);
            glTexCoord2f(0, 1);
            glVertex3f(-1, 1, 0);
        glEnd();
    }
}

void renderSpacecraft()
{
    glUseProgram(programs[1]);

    model.LoadIdentity();
    model.Rotate(10, 0, 1, 0);
    model.Translate(0, 11, 0);

    int loc = glGetUniformLocation(programs[1], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

    loc = glGetUniformLocation(programs[1], "normal_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, copyNormal());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex[0]);
    glEnable(GL_TEXTURE_2D);

    glBindVertexArray(vaos[0]);
    glDrawElements(GL_TRIANGLES, objects[0].faceCnt * 3, GL_UNSIGNED_INT, NULL);
}

void renderMissile()
{
    if (missile_active == false)
        return;

    glUseProgram(programs[1]);

    model.LoadIdentity();
    model.Scale(0.5f, 0.5f, 0.5f);
    model.Translate(0, missile_position, -3);
    model.Rotate(-missile_angle, 0, 1, 0);

    int loc = glGetUniformLocation(programs[1], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

    loc = glGetUniformLocation(programs[1], "normal_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, copyNormal());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex[2]);
    glEnable(GL_TEXTURE_2D);

    glBindVertexArray(vaos[3]);
    glDrawElements(GL_TRIANGLES, objects[3].faceCnt * 3, GL_UNSIGNED_INT, NULL);

    missile_angle = 0 ? (missile_angle + 3 >= 360) : missile_angle + 3;
    missile_position -= 0.3f;

    if (missile_position < -100)
    {
        missile_active = false;
        missile_position = 22;
        std::cout << "Missile Ready!" << std::endl;
    }
}

void renderScene()
{
    float* normal_matrix = new float[9];

    view.LoadIdentity();
    view.Translate(px, py, -35 + pz);
    view.Rotate(90, 1, 0, 0);
    view.Rotate(world_angle, 0, 0, 1);
    world_angle = 0 ? ( world_angle + 0.2f * rotation * rotation >= 360) : world_angle + 0.2f * rotation * rotation;

    for (int i = 0; i < 4; i++)
    {
        glUseProgram(programs[i]);
        int loc = glGetUniformLocation(programs[i], "view_matrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, view.matrix);
    }

    renderFighters(1);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    renderBackground();
    renderFighters(2);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderParticles();
    updateParticles();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    renderSpacecraft();
    renderMissile();
    renderUFO();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void setTextures()
{
    glGenTextures(3, color_tex);
    const char* files[4] = {"models/craft_color.bmp", "models/UFO_color.bmp", "models/missile_color.bmp", "models/bg.bmp"};
    for (int i = 0; i < 4; i++)
    {
        unsigned char* data = loadBMP(files[i]);
        glBindTexture(GL_TEXTURE_2D, color_tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

void setShaders()
{
    programs[0] = createShaderProgram((char*)"./shaders/fighter.vert", (char*)"./shaders/fighter.frag");
    programs[1] = createShaderProgram((char*)"./shaders/spacecraft.vert", (char*)"./shaders/spacecraft.frag");
    programs[2] = createShaderProgram((char*)"./shaders/ufo.vert", (char*)"./shaders/ufo.frag");
    programs[3] = createShaderProgram((char*)"./shaders/particles.vert", (char*)"./shaders/particles.frag");
    programs[4] = createShaderProgram((char*)"./shaders/background.vert", (char*)"./shaders/background.frag");

    Matrix4x4 projection;
    projection.setPerspective(30, 1.0f, 1.0f, 100.0f);
    for (int i = 0; i < 4; i++)
    {
        glUseProgram(programs[i]);
        int loc = glGetUniformLocation(programs[i], "projection_matrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);

        glUseProgram(programs[i]);
        loc = glGetUniformLocationARB(programs[i], "color_tex");
        glUniform1iARB(loc, 0);
    }
}

void initialize()
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1000, 1000, "Space Battle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, keyCallback);

    glfwMakeContextCurrent(window);

    if (GLEW_OK != glewInit())
        exit(EXIT_FAILURE);

    glfwSwapInterval(1);
}

int main() {
    initialize();

    loadObject(objects[0], "models/spacecraft.dae");
    loadObject(objects[1], "models/ufo.dae");
    loadObject(objects[2], "models/fighter.ply");
    loadObject(objects[3], "models/missile.ply");

    setShaders();
    setTextures();
    setVertexArray();
    setFBO(1000, 1000);
    setParticles();

    while (!glfwWindowShouldClose(window))
        renderScene();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}