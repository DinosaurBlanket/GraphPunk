#include <iostream>
using std::cout;
using std::endl;
#include <GL/freeglut.h>
#include "view_map.hpp"
#include "ctrl_scrollable.hpp"

float pointDevButtonState  = 0;
float pPointDevButtonState = 0;
vec2  pointDevPos;
vec2  pPointDevPos;

void pointDevButtonGCB(int button, int state, int x, int y) {
  pPointDevButtonState = pointDevButtonState;
  pointDevButtonState = !state;//result should be 1 when pressed, 0 when not
  pPointDevPos = pointDevPos;
  pointDevPos = vec2(x, y);
  view_map_scroll(
    pointDevButtonState,
    pPointDevButtonState,
    pointDevPos,
    pPointDevPos
  );
  glutPostRedisplay();
  //cout << "pointDeviceButtonGCB: " << endl
  //<< "\tbutton: " << button << endl
  //<< "\tstate : " << state << endl
  //<< "x: " << x << " y: " << y << endl;
}
void pointDevDragGCB(int x, int y) {
  pPointDevButtonState = pointDevButtonState;
  pointDevButtonState = 1;
  pPointDevPos = pointDevPos;
  pointDevPos = vec2(x, y);
  view_map_scroll(
    pointDevButtonState,
    pPointDevButtonState,
    pointDevPos,
    pPointDevPos
  );
  glutPostRedisplay();
  //cout << "pointDeviceDragGCB: " << "x: " << x << " y: " << y << endl;
}
