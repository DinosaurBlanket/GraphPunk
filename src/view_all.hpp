#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include "options.h"

#if GLEC_ENABLED
void glec(const int line, const char *file);
#define _glec glec(__LINE__, __FILE__);
#else
#define _glec
#endif

const char *shaderTypeString(GLuint st);

void viewInit(int argc, char** argv);

void addShader(
  GLuint      shaderProgram,
  const char* shaderText,
  GLenum      shaderType
);

void compileShaders(
  GLuint     &shaderProgram,
  const char *vsPath,
  const char *fsPath
);

extern vec2  screenSize;
extern vec4  gridRect;
extern float gridUnit;
extern mat4  scaledTransform;
extern mat4  scrolledTransform;

