#version 330

uniform vec2 scroll;
uniform vec2 halfVideoSize;

in  vec2 pos;
in  vec2 texCoord;
out vec2 texCoordThru;

void main() {
  texCoordThru = texCoord/vec2(512,512);
  gl_Position = vec4((pos+scroll)/halfVideoSize, 0.0, 1.0);
}
