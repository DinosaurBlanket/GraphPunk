
#include "common.hpp"
#include "GL_util.hpp"


GLuint VBO;
GLint attr_GL_pos;
GLint attr_screen_pos;
static void grid_renderCB() {
  //glClear(GL_COLOR_BUFFER_BIT);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glEnableVertexAttribArray(attr_GL_pos);_glec
  glEnableVertexAttribArray(attr_screen_pos);_glec
  glVertexAttribPointer(attr_GL_pos,     3, GL_FLOAT, GL_FALSE, 5*sizeof(float), NULL);_glec
  glVertexAttribPointer(attr_screen_pos, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (const GLvoid*)(3*sizeof(float)));_glec
  glDrawArrays(GL_TRIANGLES, 0, 3);_glec
  glDisableVertexAttribArray(attr_GL_pos);_glec
  glDisableVertexAttribArray(attr_screen_pos);_glec
  glutSwapBuffers();
}




GLuint grid_shader;
const char* grid_vsPath = "grid_vs.glsl";
const char* grid_fsPath = "grid_fs.glsl";
const int screenWidth = 1280;
const int screenHeight = 800;
const int gridUnit = 16;


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
  GLint unif_gridUnit  = glGetUniformLocation(grid_shader, "gridUnit");_glec
  glUniform1i(unif_gridUnit, gridUnit);_glec
  attr_GL_pos     = glGetAttribLocation(grid_shader, "GL_pos");_glec
  attr_screen_pos = glGetAttribLocation(grid_shader, "screen_pos");_glec
  float vertexData[] = {
    -1,  3, 0,  0,           0-screenHeight,
     3, -1, 0,  screenWidth*2, screenHeight,
    -1, -1, 0,  0,             screenHeight
  };
  glGenBuffers(1, &VBO);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);_glec
  
  glutMainLoop();
  
  exit(0);
}
