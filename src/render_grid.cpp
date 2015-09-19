#include "GL_util.hpp"
#include "render_grid.hpp"

GLuint VBO;
GLuint grid_shader;
GLint  attr_GL_pos;
GLint  attr_screen_pos;
GLint  unif_gridUnit;
const char* grid_vsPath = "src/grid_vs.glsl";
const char* grid_fsPath = "src/grid_fs.glsl";
const float vertexData[] = { -1, 3, 0,    3, -1, 0,    -1, -1, 0 };

void render_grid_init(const int gridUnit) {
  compileShaders(grid_shader, grid_vsPath, grid_fsPath);
  unif_gridUnit = glGetUniformLocation(grid_shader, "gridUnit");_glec
  glUniform1i(unif_gridUnit, gridUnit);_glec
  attr_GL_pos = glGetAttribLocation(grid_shader, "GL_pos");_glec
  glGenBuffers(1, &VBO);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(vertexData),
    vertexData,
    GL_STATIC_DRAW
  );_glec
}

void render_grid_draw() {
  glUseProgram(grid_shader);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glEnableVertexAttribArray(attr_GL_pos);_glec
  glVertexAttribPointer(
    attr_GL_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), NULL
  );_glec
  glDrawArrays(GL_TRIANGLES, 0, 3);_glec
  glDisableVertexAttribArray(attr_GL_pos);_glec
}

