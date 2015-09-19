
#include "common.hpp"
#include "GL_util.hpp"
#include "render_grid.hpp"

static void glutCB_display() {
  //glClear(GL_COLOR_BUFFER_BIT);_glec
  render_grid_draw();
  glutSwapBuffers();
}

const int screenW = 1280;
const int screenH = 800;
const int gridUnit = 16;
const char* grid_vsPath = "src/grid_vs.glsl";
const char* grid_fsPath = "src/grid_fs.glsl";


int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
  glutInitWindowSize(screenW, screenH);
  glutInitWindowPosition(160, 50);
  glutCreateWindow("GraphPunk");
  glutDisplayFunc(glutCB_display);
  
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    cout << "Error: " << endl << glewGetErrorString(res) << endl;
    return 1;
  }
  
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  
  //glClearColor(0,0,0,0);
  
  render_grid_init(grid_vsPath, grid_fsPath, screenW, screenH, gridUnit);
  
  glutMainLoop();
  
  exit(0);
}
