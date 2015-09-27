#include <iostream>
using std::cout;
using std::endl;
#include "view_map.hpp"
#include "ctrl_scrollable.hpp"

float pointDevButtonState  = 0;
float pPointDevButtonState = 0;
vec2  pointDevPos;
vec2  pPointDevPos;

void pointDevButtonGCB(int button, int state, int x, int y) {
  pPointDevButtonState = pointDevButtonState;
  pointDevButtonState = state;
  view_map_scroll(
    pointDevButtonState,
    pPointDevButtonState,
    pointDevPos,
    pPointDevPos
  );
  //cout << "pointDeviceButtonGCB: " << endl
  //<< "\tbutton: " << button << endl
  //<< "\tstate : " << state << endl
  //<< "x: " << x << " y: " << y << endl;
}
void pointDevDragGCB(int x, int y) {
  pPointDevPos = pointDevPos;
  pointDevPos = vec2(x, y);
  view_map_scroll(
    pointDevButtonState,
    pPointDevButtonState,
    pointDevPos,
    pPointDevPos
  );
  //cout << "pointDeviceDragGCB: " << "x: " << x << " y: " << y << endl;
}
