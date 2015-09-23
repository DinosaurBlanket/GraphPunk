
#include <iostream>
using std::cout;
using std::endl;
#include "view_map.hpp"
#include <vector>

GLuint VBO;
GLuint IBO;
GLuint shaderProgram;
GLint attr_pos;
GLint attr_color;
GLint unif_transform;
const char *vsPath = "src/view_map_vs.glsl";
const char *fsPath = "src/view_map_fs.glsl";
struct vertex {
  vec3 pos;
  uint32_t  color;
};
std::vector<vertex>  vertexData;
std::vector<uint16_t> indexData;
mat4 transform;

#if LOGGING_ENABLED
#include <iostream>
#include <fstream>
void logVertexData() {
  std::ofstream of;
  of.open("log/vertexData.txt");
  for (uint32_t i = 0; i < vertexData.size(); i ++) {
    of <<
    vertexData[i].pos.x << "\t" <<
    vertexData[i].pos.y << "\t" <<
    vertexData[i].pos.z << "\t" << endl;
  }
  of.close();
}
#endif


void view_map_init(vec4 gridRect, vec2 screenSize, float gridUnit) {
  {
    const float    halfGridLine  = 1.0/gridUnit;//grid units
    const uint32_t gridLineColor = 0xff886622;//ABGR
    const int hrzlGridLineVrtxCount = (gridRect[2] + 1) * 4;
    const int vrtlGridLineVrtxCount = (gridRect[3] + 1) * 4;
    const int gridLineVrtxCount = hrzlGridLineVrtxCount + vrtlGridLineVrtxCount;
    vertexData.resize(gridLineVrtxCount);
    int i, v;
    float x, y;
    for (i = 0, x = 0; i < hrzlGridLineVrtxCount; i += 4, x++) {
      vertexData[i  ].color = gridLineColor;
      vertexData[i  ].pos = vec3(
        x+gridRect[0]-halfGridLine, gridRect[1],             layer_gridLines
      );
      vertexData[i+1].color = gridLineColor;
      vertexData[i+1].pos = vec3(
        x+gridRect[0]+halfGridLine, gridRect[1],             layer_gridLines
      );
      vertexData[i+2].color = gridLineColor;
      vertexData[i+2].pos = vec3(
        x+gridRect[0]-halfGridLine, gridRect[1]-gridRect[3], layer_gridLines
      );
      vertexData[i+3].color = gridLineColor;
      vertexData[i+3].pos = vec3(
        x+gridRect[0]+halfGridLine, gridRect[1]-gridRect[3], layer_gridLines
      );
    }
    for (i = hrzlGridLineVrtxCount, y = 0; i < gridLineVrtxCount; i += 4, y++) {
      vertexData[i  ].color = gridLineColor;
      vertexData[i  ].pos = vec3(
        gridRect[0],             (y-gridRect[1])-halfGridLine, layer_gridLines
      );
      vertexData[i+1].color = gridLineColor;
      vertexData[i+1].pos = vec3(
        gridRect[0]+gridRect[2], (y-gridRect[1])-halfGridLine, layer_gridLines
      );
      vertexData[i+2].color = gridLineColor;
      vertexData[i+2].pos = vec3(
        gridRect[0],             (y-gridRect[1])+halfGridLine, layer_gridLines
      );
      vertexData[i+3].color = gridLineColor;
      vertexData[i+3].pos = vec3(
        gridRect[0]+gridRect[2], (y-gridRect[1])+halfGridLine, layer_gridLines
      );
    }
    #if LOGGING_ENABLED
    logVertexData();
    #endif
    const int indexCount = gridLineVrtxCount*1.5;
    indexData.resize(indexCount);
    for (i = 0, v = 0; i < indexCount; i += 6, v += 4) {
      indexData[i  ] = v;
      indexData[i+1] = v + 1;
      indexData[i+2] = v + 2;
      indexData[i+3] = v + 1;
      indexData[i+4] = v + 2;
      indexData[i+5] = v + 3;
    }
  }
  transform = scale(
    mat4(), vec3((gridUnit*2)/screenSize.x, (gridUnit*2)/screenSize.y, 1.0)
  );
  compileShaders(shaderProgram, vsPath, fsPath);
  unif_transform = glGetUniformLocation(shaderProgram, "transform");_glec
  glUniformMatrix4fv(
    unif_transform, 1, GL_FALSE, (const GLfloat*)&transform
  );_glec
  attr_pos   = glGetAttribLocation(shaderProgram, "pos");_glec
  attr_color = glGetAttribLocation(shaderProgram, "color");_glec
  glGenBuffers(1, &VBO);_glec
  glGenBuffers(1, &IBO);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);_glec
  glBufferData(
    GL_ARRAY_BUFFER,
    vertexData.size() * sizeof(vertex),
    (const GLvoid*)vertexData.data(),
    GL_STATIC_DRAW //GL_DYNAMIC_DRAW might be better, not sure yet
  );_glec
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    indexData.size() * sizeof(GL_UNSIGNED_SHORT),
    (const GLvoid*)indexData.data(),
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
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);_glec
  glEnableVertexAttribArray(attr_pos);_glec
  glEnableVertexAttribArray(attr_color);_glec
  glVertexAttribPointer(
    attr_pos,   3, GL_FLOAT,         false, 16, (const GLvoid*) 0
  );_glec
  glVertexAttribPointer(
    attr_color, 4, GL_UNSIGNED_BYTE, true,  16, (const GLvoid*)12
  );_glec
  glDrawElements(
    GL_TRIANGLES, indexData.size(), GL_UNSIGNED_SHORT, (const GLvoid*)0
  );_glec
  glDisableVertexAttribArray(attr_pos);_glec
  glDisableVertexAttribArray(attr_color);_glec
}


