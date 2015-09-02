#version 130

in vec3 GL_pos;
in vec2 screen_pos;
out vec2 screen_pos_terp;

void main() {
  gl_Position = vec4(GL_pos.x, GL_pos.y, GL_pos.z, 1.0);
  screen_pos_terp = screen_pos;
}
