#include <iostream>
using std::cout;
using std::endl;

void pointDeviceButtonGCB(int button, int state, int x, int y) {
  cout << "pointDeviceButtonGCB: " << endl
  << "\tbutton: " << button << endl
  << "\tstate : " << state << endl
  << "x: " << x << " y: " << y << endl;
}
void pointDeviceDragGCB(int x, int y) {
  cout << "pointDeviceDragGCB: " << "x: " << x << " y: " << y << endl;
}
