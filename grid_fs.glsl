#version 130

uniform int screenWidth;
uniform int screenHeight;

in  vec2 screen_pos_terp;
out vec4 FragColor;

void main() {
  FragColor = vec4(
    0,
    screen_pos_terp.x/screenWidth,
    screen_pos_terp.y/screenHeight,
    1
  );
}
