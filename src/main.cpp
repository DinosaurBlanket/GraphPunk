
#include <iostream>
using std::cout;
using std::endl;
#include "view_map.hpp"

static void glutCB_display() {
  //glClear(GL_COLOR_BUFFER_BIT);_glec
  view_map_draw();
  glutSwapBuffers();
}

vec2  screenSize = vec2(1280, 800);//pixels
vec4  gridRect   = vec4(-80, 50, 160, 100);//grid units
float gridUnit   = 16;//pixels

int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
  glutInitWindowSize(screenSize.x, screenSize.y);
  glutInitWindowPosition(160, 50);
  glutCreateWindow("GraphPunk");
  glutDisplayFunc(glutCB_display);
  
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    cout << "Error: " << endl << glewGetErrorString(res) << endl;
    return 1;
  }
  
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  
  glClearColor(0.0,0.2,0.3,1.0);_glec
  
  view_map_init(gridRect, screenSize, gridUnit);
  
  glutMainLoop();
  
  exit(0);
}
