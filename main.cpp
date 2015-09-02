
#include <iostream>
using std::cout;
using std::endl;
#include <cstring>
#include <string>
using std::string;
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec3.hpp>
using glm::vec3;

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

void glec(int line, const char *file) {
  GLenum GLstatus;
  while ((GLstatus = glGetError()) != GL_NO_ERROR) {
    cout << "OpenGL error: " << GLstatus << " on line " << line << " in " << file << endl;
  }
}
#define _glec glec(__LINE__, __FILE__)

const char *shaderTypeString(GLuint st) {
  switch(st) {
    case GL_VERTEX_SHADER  : return "vertex shader";
    case GL_FRAGMENT_SHADER: return "fragment shader";
  }
  return NULL;
}

static void addShader(
  GLuint      shaderProgram,
  const char* shaderText,
  GLenum      shaderType
) {
  GLuint shaderObj = glCreateShader(shaderType);_glec;
  if (!shaderObj) {
    cout << "Error creating " << shaderTypeString(shaderType) << endl;
    exit(__LINE__);
  }
  GLint shaderTextLen = strlen(shaderText);
  glShaderSource(shaderObj, 1, &shaderText, &shaderTextLen);_glec;
  glCompileShader(shaderObj);_glec;
  GLint success;
  glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);_glec;
  if (!success) {
    GLchar infoLog[1024];
    glGetShaderInfoLog(shaderObj, sizeof(infoLog), NULL, infoLog);_glec;
    cout << "Error compiling " << shaderTypeString(shaderType) << endl
    << infoLog << endl;
    exit(__LINE__);
  }
  glAttachShader(shaderProgram, shaderObj);_glec;
}

static void compileShaders(
  GLuint     &shaderProgram,
  const char *vsPath,
  const char *fsPath
) {
  shaderProgram = glCreateProgram();_glec;
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
  glLinkProgram(shaderProgram);_glec;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);_glec;
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);_glec;
    cout << "Error linking shader program: " << endl << errorLog << endl;
    exit(__LINE__);
  }
  glValidateProgram(shaderProgram);_glec;
  glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);_glec;
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);_glec;
    cout << "Invalid shader program: " << endl << errorLog << endl;
    exit(__LINE__);
  }
  glUseProgram(shaderProgram);_glec;
}

GLuint VBO;
GLint attr_GL_pos;
GLint attr_screen_pos;
static void grid_renderCB() {
  glClear(GL_COLOR_BUFFER_BIT);_glec;
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec;
  glEnableVertexAttribArray(attr_GL_pos);_glec;
  glEnableVertexAttribArray(attr_screen_pos);_glec;
  glVertexAttribPointer(attr_GL_pos,     3, GL_FLOAT, GL_FALSE, 5*sizeof(float), NULL);_glec;
  glVertexAttribPointer(attr_screen_pos, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (const GLvoid*)(3*sizeof(float)));_glec;
  glDrawArrays(GL_TRIANGLES, 0, 3);_glec;
  glDisableVertexAttribArray(attr_GL_pos);_glec;
  glDisableVertexAttribArray(attr_screen_pos);_glec;
  glutSwapBuffers();
}

GLuint grid_shader;
const char* grid_vsPath = "grid_vs.glsl";
const char* grid_fsPath = "grid_fs.glsl";
const int screenWidth = 1280;
const int screenHeight = 800;


int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
  glutInitWindowSize(screenWidth, screenHeight);
  glutInitWindowPosition(160, 50);
  glutCreateWindow("GraphPunk");
  glutDisplayFunc(grid_renderCB);
  
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    cout << "Error: " << endl << glewGetErrorString(res) << endl;
    return 1;
  }
  
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  
  glClearColor(0,0,0,0);
  
  compileShaders(grid_shader, grid_vsPath, grid_fsPath);
  GLint unif_screenWidth  = glGetUniformLocation(grid_shader, "screenWidth");_glec;
  GLint unif_screenHeight = glGetUniformLocation(grid_shader, "screenHeight");_glec;
  glUniform1i(unif_screenWidth, screenWidth);_glec;
  glUniform1i(unif_screenHeight, screenHeight);_glec;
  attr_GL_pos     = glGetAttribLocation(grid_shader, "GL_pos");_glec;
  attr_screen_pos = glGetAttribLocation(grid_shader, "screen_pos");_glec;
  float vertexData[] = {
    -1,  3, 0,  0,           0-screenHeight,
     3, -1, 0,  screenWidth*2, screenHeight,
    -1, -1, 0,  0,             screenHeight
  };
  glGenBuffers(1, &VBO);_glec;
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec;
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);_glec;
  
  glutMainLoop();
  
  exit(0);
}
