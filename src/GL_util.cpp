#include <iostream>
using std::cout;
using std::endl;
#include "GL_util.hpp"

#include <cstring>
#include <string>
using std::string;
#include <sstream>
#include <fstream>
int readFile(const char *path, string &out) {
  std::ifstream fs(path);
  std::stringstream ss;
  ss << fs.rdbuf();
  if (fs.fail()) {cout << "Error reading file: " << path << endl;}
  out = ss.str();
  return out.length();
}
#if GLEC_ENABLED
void glec(const int line, const char *file) {
  GLenum GLstatus;
  while ((GLstatus = glGetError()) != GL_NO_ERROR) {
    cout<<"OpenGL error: "<<GLstatus<<" on line "<<line<<" in "<<file<<endl;
  }
}
#endif

const char *shaderTypeString(GLuint st) {
  switch(st) {
    case GL_VERTEX_SHADER  : return "vertex shader";
    case GL_FRAGMENT_SHADER: return "fragment shader";
  }
  return "unknown shader type";
}

void addShader(
  GLuint      shaderProgram,
  const char* shaderText,
  GLenum      shaderType
) {
  GLuint shaderObj = glCreateShader(shaderType);_glec
  if (!shaderObj) {
    cout << "Error creating " << shaderTypeString(shaderType) << endl;
    exit(__LINE__);
  }
  GLint shaderTextLen = strlen(shaderText);
  glShaderSource(shaderObj, 1, &shaderText, &shaderTextLen);_glec
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
  if (!shaderProgram) {
    cout << "Error creating shader program" << endl;
    exit(__LINE__);
  }
  string vs, fs;
  if (!readFile(vsPath, vs)) exit(__LINE__);
  if (!readFile(fsPath, fs)) exit(__LINE__);
  addShader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);
  addShader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);
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
  glUseProgram(shaderProgram);_glec
}
