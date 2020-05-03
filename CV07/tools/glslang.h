#include "GL/glew.h"

GLuint createShaderProgram1(char *vertex, char *fragment) ;
GLuint createShaderProgram(char *vertex, char *geometry, char *fragment);
char *textFileRead(char *fn) ;