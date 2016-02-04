#version 330

uniform vec2 unitScale;
//uniform vec2 scroll;
in      vec2 pos;
in      vec4 color;
out     vec4 colorToFrag;

void main() {
  colorToFrag = color;
  gl_Position = vec4(pos*unitScale, 0.0, 1.0);
}
