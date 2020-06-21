#include "GL/glew.h"

GLuint createShaderProgram(char *vertex, char *fragment) ;
GLuint createShaderProgram1(char *vertex, char *geometry, char *fragment);
char *textFileRead(char *fn) ;