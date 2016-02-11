#version 330

uniform vec2 unitScale;
uniform vec2 scroll;

in  vec2 pos;
in  vec2 texCoord;
out vec2 texCoordThru;

void main() {
  texCoordThru = texCoord;
  gl_Position = vec4(pos*unitScale + scroll, 0.0, 1.0);
}
