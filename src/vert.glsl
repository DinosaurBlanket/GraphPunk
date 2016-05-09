#version 330

uniform vec2 halfVideoSize;
uniform vec2 texSize;
uniform vec2 scroll;

in  vec2 pos;
in  vec2 texCoord;
out vec2 texCoordThru;

void main() {
  texCoordThru = texCoord/texSize;
  gl_Position = vec4((pos+scroll)/halfVideoSize, 0.0, 1.0);
}
