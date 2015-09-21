#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

void glec(int line, const char *file);
#define _glec glec(__LINE__, __FILE__);

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
