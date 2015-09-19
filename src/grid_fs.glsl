#version 130

#define backColor vec4(0.0,0.2,0.3,1.0)
#define lineColor vec4(0.1,0.4,0.5,1.0)

uniform int gridUnit;
out vec4 FragColor;

void main() {
  int ofGridUnitX = int(gl_FragCoord.x) % gridUnit;
  int ofGridUnitY = int(gl_FragCoord.y) % gridUnit;
  bool isLine =
    ofGridUnitX == 0          ||
    ofGridUnitX == gridUnit-1 ||
    ofGridUnitY == 0          ||
    ofGridUnitY == gridUnit-1
  ;
  FragColor = isLine ? lineColor : backColor;
}
