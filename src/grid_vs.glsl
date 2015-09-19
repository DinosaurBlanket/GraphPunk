#version 130
in vec3 GL_pos;
void main() {
  gl_Position = vec4(GL_pos, 1.0);
}
