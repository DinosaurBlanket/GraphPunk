#version 130

#define backColor vec4(0.0,0.2,0.3,1.0)
#define lineColor vec4(0.1,0.4,0.5,1.0)

uniform int gridUnit;

in  vec2 screen_pos_terp;
out vec4 FragColor;

void main() {
  /*
  int2 ofGridUnit = int2(
    int(screen_pos_terp.x) % gridUnit,
    int(screen_pos_terp.y) % gridUnit
  );
  bool isLine =
    ofGridUnit.x == 0 ||
    ofGridUnit.x == 1 ||
    ofGridUnit.y == 0 ||
    ofGridUnit.y == 1
  ;
  */
  int ofGridUnitX = int(screen_pos_terp.x) % gridUnit;
  int ofGridUnitY = int(screen_pos_terp.y) % gridUnit;
  bool isLine =
    ofGridUnitX ==  0 ||
    ofGridUnitX == 15 ||
    ofGridUnitY ==  0 ||
    ofGridUnitY == 15
  ;
  FragColor = isLine ? lineColor : backColor;
}
