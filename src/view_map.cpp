#include "view_map.hpp"
#include "GL_util.hpp"
#include <vector>

GLuint VBO;
GLuint shaderProgram;
GLint attr_pos;
GLint attr_color;
GLint unif_transform;
const char *vsPath = "src/view_map_vs.glsl";
const char *fsPath = "src/view_map_fs.glsl";
struct vertex {
  glm::vec3 pos;
  uint32_t  color;
};
std::vector<vertex>  vertexData;
std::vector<uint32_t> indexData;
glm::mat4 transform = glm::mat4();


void view_map_init(glm::vec4 gridRect) {
  vertexData.resize(4);
  vertexData[0].pos = glm::vec3(-0.5,  0.5, 0);
  vertexData[1].pos = glm::vec3( 0.5,  0.5, 0);
  vertexData[2].pos = glm::vec3(-0.5, -0.5, 0);
  vertexData[3].pos = glm::vec3( 0.5, -0.5, 0);
  vertexData[0].color = 0xffffffff;//ABGR
  vertexData[1].color = 0xffff0000;
  vertexData[2].color = 0xff00ff00;
  vertexData[3].color = 0xff0000ff;
  indexData.resize(6);
  indexData[0] = 0;
  indexData[1] = 1;
  indexData[2] = 2;
  indexData[3] = 1;
  indexData[4] = 2;
  indexData[5] = 3;
  
  compileShaders(shaderProgram, vsPath, fsPath);
  unif_transform = glGetUniformLocation(shaderProgram, "transform");_glec
  glUniformMatrix4fv(unif_transform, 1, GL_FALSE, (const GLfloat*)&transform);_glec
  attr_pos   = glGetAttribLocation(shaderProgram, "pos");_glec
  attr_color = glGetAttribLocation(shaderProgram, "color");_glec
  glGenBuffers(1, &VBO);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glBufferData(
    GL_ARRAY_BUFFER,
    vertexData.size() * sizeof(vertex),
    (const GLvoid*)vertexData.data(),
    GL_STATIC_DRAW //GL_DYNAMIC_DRAW might be better, not sure yet
  );_glec
}

void view_map_resize();
void view_map_addPatch();
void view_map_remPatch();
void view_map_repPatch();

void view_map_draw() {
  glClear(GL_COLOR_BUFFER_BIT);_glec
  glUseProgram(shaderProgram);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glEnableVertexAttribArray(attr_pos);_glec
  glEnableVertexAttribArray(attr_color);_glec
  glVertexAttribPointer(
    attr_pos,   3, GL_FLOAT,         false, 16, (const GLvoid*) 0
  );_glec
  glVertexAttribPointer(
    attr_color, 4, GL_UNSIGNED_BYTE, true,  16, (const GLvoid*)12
  );_glec
  glDrawElements(
    GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const GLvoid*)indexData.data()
  );_glec
  glDisableVertexAttribArray(attr_pos);_glec
  glDisableVertexAttribArray(attr_color);_glec
}


