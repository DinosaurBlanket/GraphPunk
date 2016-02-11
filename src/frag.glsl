#version 330

uniform sampler2D tex;

in  vec2 texCoordThru;
out vec4 outColor;

void main() {
  outColor = texture(tex, texCoordThru);
}
