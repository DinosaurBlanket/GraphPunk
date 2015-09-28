#include <iostream>
using std::cout;
using std::endl;
#include "view_all.hpp"
#include "view_map.hpp"
#include "ctrl_scrollable.hpp"
#include "ctrl_point.hpp"

//int prevGlutElapsedTime = 0;
//int crntGlutElapsedTime = 0;
void displayGCB() {
  //prevGlutElapsedTime = crntGlutElapsedTime;
  //crntGlutElapsedTime = glutGet(GLUT_ELAPSED_TIME);
  //int deltaT = crntGlutElapsedTime - prevGlutElapsedTime;
  glClear(GL_COLOR_BUFFER_BIT);_glec
  view_map_draw();
  glutSwapBuffers();
}

vec2  screenSize;
vec4  gridRect;
float gridUnit;
mat4  scaledTransform;
mat4  scrolledTransform;

void viewInit(int argc, char** argv) {
  screenSize = vec2(640, 480);//pixels
  gridRect   = vec4(-40, 30, 80, 60);//grid units
  gridUnit   = 16;//pixels
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
  glutInitWindowSize(screenSize.x, screenSize.y);
  glutCreateWindow("GraphPunk");
  glutDisplayFunc(displayGCB);
  glutMouseFunc(pointDevButtonGCB);
  glutMotionFunc(pointDevDragGCB);
  
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    cout << "Error: " << endl << glewGetErrorString(res) << endl;
    return;
  }
  
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  
  glClearColor(0.0,0.2,0.3,1.0);_glec
  view_map_init();
}

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
