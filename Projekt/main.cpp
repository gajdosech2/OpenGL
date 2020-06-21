#define _XKEYCHECK_H
#define GLFW_INCLUDE_NONE
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_DEPRECATE

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

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
ObjectData objects[4];
GLuint color_tex[3];
GLuint programs[3];
GLuint vaos[4];
Matrix4x4 model;
Matrix4x4 view;

float world_angle = 0;
float ufo_angle = 0;

void loadScene(ObjectData& od, const std::string file)
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
}

void background()
{
    glUseProgram(programs[2]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex[2]);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
        glNormal3f(0.0, 0.0, 1.0);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(-1.0, -1.0, 0.0);

        glTexCoord2f(1.0, 0.0);
        glVertex3f(1.0, -1.0, 0.0);

        glTexCoord2f(1.0, 1.0);
        glVertex3f(1.0, 1.0, 0.0);

        glTexCoord2f(0.0, 1.0);
        glVertex3f(-1.0, 1.0, 0.0);
    glEnd();
}

void renderScene()
{
    int loc;
    float* normal_matrix = new float[9];

    view.LoadIdentity();
    view.Translate(0, 0, -35);
    view.Rotate(90, 1, 0, 0);
    view.Rotate(world_angle, 0, 0, 1);

    world_angle = 0 ? ( world_angle + 0.2f >= 360) : world_angle + 0.2f;

    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    background();

    glEnable(GL_DEPTH_TEST);

    glUseProgram(programs[0]);

    loc = glGetUniformLocation(programs[0], "view_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, view.matrix);

    glUseProgram(programs[1]);

    loc = glGetUniformLocation(programs[1], "view_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, view.matrix);

    //SpaceCraft
    glUseProgram(programs[1]);

    model.LoadIdentity();
    model.Rotate(10, 0, 1, 0);
    model.Translate(0, 11, 0);
    
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            normal_matrix[j + 3 * i] = model.matrix[j + 4 * i];

    loc = glGetUniformLocation(programs[1], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

    loc = glGetUniformLocation(programs[1], "normal_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, normal_matrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex[1]);
    glEnable(GL_TEXTURE_2D);

    glBindVertexArray(vaos[0]);
    glDrawElements(GL_TRIANGLES, objects[0].faceCnt * 3, GL_UNSIGNED_INT, NULL);

    //SpaceFighters
    glUseProgram(programs[0]);

    model.LoadIdentity();
    model.Translate(10, 10, -4);
    model.Rotate(-70, 0, 0, 1);
    model.Scale(0.3f, 0.3f, 0.3f);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            normal_matrix[j + 3 * i] = model.matrix[j + 4 * i];

    loc = glGetUniformLocation(programs[0], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

    loc = glGetUniformLocation(programs[0], "normal_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, normal_matrix);

    glBindVertexArray(vaos[2]);
    glDrawElements(GL_TRIANGLES, objects[2].faceCnt * 3, GL_UNSIGNED_INT, NULL);

    model.LoadIdentity();
    model.Translate(-11, 4, 7);
    model.Rotate(-120, 0, 0, 1);
    model.Scale(0.3f, 0.3f, 0.3f);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            normal_matrix[j + 3 * i] = model.matrix[j + 4 * i];

    loc = glGetUniformLocation(programs[0], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

    loc = glGetUniformLocation(programs[0], "normal_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, normal_matrix);

    glBindVertexArray(vaos[2]);
    glDrawElements(GL_TRIANGLES, objects[2].faceCnt * 3, GL_UNSIGNED_INT, NULL);

    //UFO
    glUseProgram(programs[1]);

    model.LoadIdentity();
    model.Translate(0, -13, 3);
    model.Scale(1.5f, 1.5f, 1.5f);
    model.Rotate(ufo_angle, 0, 0, 1);
    model.Rotate(10, 0, 1, 0);

    ufo_angle = 0 ? (ufo_angle + 2 >= 360) : ufo_angle + 2;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            normal_matrix[j + 3 * i] = model.matrix[j + 4 * i];

    loc = glGetUniformLocation(programs[1], "model_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.matrix);

    loc = glGetUniformLocation(programs[1], "normal_matrix");
    glUniformMatrix3fv(loc, 1, GL_FALSE, normal_matrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex[0]);
    glEnable(GL_TEXTURE_2D);

    glBindVertexArray(vaos[1]);
    glDrawElements(GL_TRIANGLES, objects[1].faceCnt * 3, GL_UNSIGNED_INT, NULL);

    glfwSwapBuffers(window);
    glfwPollEvents();
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
    {
        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(1);
}

void textures()
{
    glGenTextures(3, color_tex);

    unsigned char* data = loadBMP("models/UFO_color.bmp");
    glBindTexture(GL_TEXTURE_2D, color_tex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    data = loadBMP("models/craft_color.bmp");
    glBindTexture(GL_TEXTURE_2D, color_tex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    data = loadBMP("models/bg.bmp");
    glBindTexture(GL_TEXTURE_2D, color_tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

int main() {
    initialize();

    loadScene(objects[0], "models/spacecraft.dae");
    loadScene(objects[1], "models/ufo.dae");
    loadScene(objects[2], "models/fighter.ply");

    Matrix4x4 projection;
    projection.setPerspective(30, 1.0f, 1.0f, 100.0f);

    programs[0] = createShaderProgram((char*)"./shaders/plain.vert", (char*)"./shaders/plain.frag");
    glUseProgram(programs[0]);

    int loc = glGetUniformLocation(programs[0], "projection_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);


    programs[1] = createShaderProgram((char*)"./shaders/textured.vert", (char*)"./shaders/textured.frag");
    glUseProgram(programs[1]);

    loc = glGetUniformLocation(programs[1], "projection_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, projection.matrix);

    loc = glGetUniformLocationARB(programs[1], "color_tex");
    glUniform1iARB(loc, 0);


    programs[2] = createShaderProgram((char*)"./shaders/bg.vert", (char*)"./shaders/bg.frag");

    loc = glGetUniformLocationARB(programs[2], "color_tex");
    glUniform1i(loc, 0);

    textures();

    setVertexArray();

    while (!glfwWindowShouldClose(window))
    {
        renderScene();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}