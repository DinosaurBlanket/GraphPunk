
#include <iostream>
using std::cout;
using std::endl;
#include <GL/glew.h>

#include "readFile.hpp"
#include "errorsAndLogging.hpp"
#include "glShaderLoader.hpp"

const char *shaderTypeString(GLuint st) {
  switch(st) {
    case GL_VERTEX_SHADER  : return "vertex shader";
    case GL_FRAGMENT_SHADER: return "fragment shader";
  }
  return "unknown shader type";
}
void addShader(
  GLuint       shaderProgram,
  std::string &shaderText,
  GLenum       shaderType
) {
  GLuint shaderObj = glCreateShader(shaderType);_glec
  GLint shaderTextLen = shaderText.size();
  const char* shaderTexts = shaderText.c_str();
  glShaderSource(shaderObj, 1, &shaderTexts, &shaderTextLen);_glec
  glCompileShader(shaderObj);_glec
  GLint success;
  glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);_glec
  if (!success) {
    GLchar infoLog[1024];
    glGetShaderInfoLog(shaderObj, sizeof(infoLog), NULL, infoLog);_glec
    cout << "Error compiling " << shaderTypeString(shaderType) << endl
    << infoLog << endl;
    exit(__LINE__);
  }
  glAttachShader(shaderProgram, shaderObj);_glec
}
void compileShaders(
  GLuint     &shaderProgram,
  const char *vsPath,
  const char *fsPath
) {
  shaderProgram = glCreateProgram();_glec
  std::string vs, fs;
  if (!readFile(vsPath, vs)) exit(__LINE__);
  if (!readFile(fsPath, fs)) exit(__LINE__);
  addShader(shaderProgram, vs, GL_VERTEX_SHADER);
  addShader(shaderProgram, fs, GL_FRAGMENT_SHADER);
  GLint success = 0;
  GLchar errorLog[1024] = {0};
  glLinkProgram(shaderProgram);_glec
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);_glec
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);_glec
    cout << "Error linking shader program: " << endl << errorLog << endl;
    exit(__LINE__);
  }
  glValidateProgram(shaderProgram);_glec
  glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);_glec
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);_glec
    cout << "Invalid shader program: " << endl << errorLog << endl;
    exit(__LINE__);
  }
}

