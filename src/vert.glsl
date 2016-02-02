#version 330
//uniform vec2 scroll;
in      vec3 pos;
in      vec4 color;
out     vec4 colorToFrag;
void main() {
  colorToFrag = color;
  gl_Position = vec4(pos, 1.0);
}
