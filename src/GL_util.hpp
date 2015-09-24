#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include "options.h"

#if GL_ERROR_CHECK
void glec(int line, const char *file);
#define _glec glec(__LINE__, __FILE__);
#else
#define _glec ;
#endif

const char *shaderTypeString(GLuint st);

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

enum layers {
  layer_gridLines,
  layer_windRose,
  layer_patches,
  layer_ports
};

