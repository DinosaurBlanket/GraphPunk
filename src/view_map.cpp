
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
  vec3     pos;
  uint32_t color;
};
typedef uint16_t index_t ;
struct vertexGroup {
  std::vector<vertex>  vrtx;
  std::vector<index_t> indx;
  uint32_t vboStart;
  uint32_t iboStart;
};
vertexGroup gridLines;
vertexGroup windRose;
std::vector<vertexGroup*> vertexGroups = {&gridLines, &windRose};
std::vector<index_t> indxAll;

uint vrtxCount = 0;
uint indxCount = 0;
mat4 transform;

#if LOGGING_ENABLED
#include <iostream>
#include <fstream>
void logVertexData(
  std::vector<vertex> &vertexData,
  uint pitch,
  const char *path
) {
  std::ofstream of;
  of.open(path);
  for (uint32_t i = 0; i < vertexData.size(); i ++) {
    of <<
    vertexData[i].pos.x << "\t" <<
    vertexData[i].pos.y << "\t" <<
    vertexData[i].pos.z << "\t\t";
    if (i % pitch == pitch-1) of << endl;
  }
  of.close();
}
void logIndexData(
  std::vector<index_t> &indexData,
  uint pitch,
  const char *path
) {
  std::ofstream of;
  of.open(path);
  for (uint32_t i = 0; i < indexData.size(); i ++) {
    of << indexData[i] << '\t';
    if (i % pitch == pitch-1) of << endl;
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
    gridLines.vrtx.resize(gridLineVrtxCount);
    int i, v;
    float x, y;
    for (i = 0, x = 0; i < hrzlGridLineVrtxCount; i += 4, x++) {
      gridLines.vrtx[i  ].color = gridLineColor;
      gridLines.vrtx[i  ].pos = vec3(
        x+gridRect[0]-halfGridLine, gridRect[1],             layer_gridLines
      );
      gridLines.vrtx[i+1].color = gridLineColor;
      gridLines.vrtx[i+1].pos = vec3(
        x+gridRect[0]+halfGridLine, gridRect[1],             layer_gridLines
      );
      gridLines.vrtx[i+2].color = gridLineColor;
      gridLines.vrtx[i+2].pos = vec3(
        x+gridRect[0]-halfGridLine, gridRect[1]-gridRect[3], layer_gridLines
      );
      gridLines.vrtx[i+3].color = gridLineColor;
      gridLines.vrtx[i+3].pos = vec3(
        x+gridRect[0]+halfGridLine, gridRect[1]-gridRect[3], layer_gridLines
      );
    }
    for (i = hrzlGridLineVrtxCount, y = 0; i < gridLineVrtxCount; i += 4, y++) {
      gridLines.vrtx[i  ].color = gridLineColor;
      gridLines.vrtx[i  ].pos = vec3(
        gridRect[0],             (y-gridRect[1])-halfGridLine, layer_gridLines
      );
      gridLines.vrtx[i+1].color = gridLineColor;
      gridLines.vrtx[i+1].pos = vec3(
        gridRect[0]+gridRect[2], (y-gridRect[1])-halfGridLine, layer_gridLines
      );
      gridLines.vrtx[i+2].color = gridLineColor;
      gridLines.vrtx[i+2].pos = vec3(
        gridRect[0],             (y-gridRect[1])+halfGridLine, layer_gridLines
      );
      gridLines.vrtx[i+3].color = gridLineColor;
      gridLines.vrtx[i+3].pos = vec3(
        gridRect[0]+gridRect[2], (y-gridRect[1])+halfGridLine, layer_gridLines
      );
    }
    #if LOGGING_ENABLED
    logVertexData(gridLines.vrtx, 4, "log/gridLinesVertexData.txt");
    #endif
    const int indexCount = gridLineVrtxCount*1.5;
    gridLines.indx.resize(indexCount);
    for (i = 0, v = 0; i < indexCount; i += 6, v += 4) {
      gridLines.indx[i  ] = v;
      gridLines.indx[i+1] = v + 1;
      gridLines.indx[i+2] = v + 2;
      gridLines.indx[i+3] = v + 1;
      gridLines.indx[i+4] = v + 2;
      gridLines.indx[i+5] = v + 3;
    }
  }
  {
    const float inr = 0.4;
    const float otr = 1.6;
    const std::vector<float> corners = {
      0, 0,      0,  otr,    inr,  inr,
      0, 0,    inr,  inr,    otr,    0,
      0, 0,    otr,    0,    inr, -inr,
      0, 0,    inr, -inr,      0, -otr,
      0, 0,      0, -otr,   -inr, -inr,
      0, 0,   -inr, -inr,   -otr,    0,
      0, 0,   -otr,    0,   -inr,  inr,
      0, 0,   -inr,  inr,      0,  otr
    };
    uint32_t colorA = 0xff114400;
    uint32_t colorB = 0xff228800;
    const int size = 24;
    windRose.vrtx.resize(size);
    windRose.indx.resize(size);
    for (int i = 0; i < size; i++) {
      windRose.vrtx[i].color = i % 6 < 3 ? colorA : colorB;
      windRose.vrtx[i].pos.x = corners[i*2];
      windRose.vrtx[i].pos.y = corners[i*2 + 1];
      windRose.vrtx[i].pos.z = layer_windRose;
      windRose.indx[i] = i;
    }
    #if LOGGING_ENABLED
    logVertexData(windRose.vrtx, 3, "log/windRoseVertexData.txt");
    logIndexData( windRose.indx, 3, "log/windRoseIndexData.txt");
    #endif
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
  for (uint i = 0; i < vertexGroups.size(); i++) {
    vrtxCount += vertexGroups[i]->vrtx.size();
    indxCount += vertexGroups[i]->indx.size();
    vertexGroups[i]->vboStart =
      !i ? 0 : vertexGroups[i-1]->vboStart + vertexGroups[i-1]->vrtx.size();
    vertexGroups[i]->iboStart =
      !i ? 0 : vertexGroups[i-1]->iboStart + vertexGroups[i-1]->indx.size();
  }
  indxAll.resize(indxCount);
  for (uint i = 0; i < vertexGroups.size(); i++) {
    for (uint j = 0; j < vertexGroups[i]->indx.size(); j++) {
      indxAll[j + vertexGroups[i]->iboStart] = vertexGroups[i]->indx[j] + (
        !i ? 0 : vertexGroups[i-1]->vboStart + vertexGroups[i-1]->vrtx.size()
      );
    }
  }
  #if LOGGING_ENABLED
  logIndexData(indxAll, 12, "log/indexAll.txt");
  #endif
  const int vboSize = sizeof(vertex )*vrtxCount;
  const int iboSize = sizeof(index_t)*indxCount;
  glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_STREAM_DRAW);_glec
  for (uint i = 0; i < vertexGroups.size(); i++) {
    glBufferSubData(
      GL_ARRAY_BUFFER,
      sizeof(vertex)*vertexGroups[i]->vboStart,
      sizeof(vertex)*vertexGroups[i]->vrtx.size(),
      (const GLvoid*)vertexGroups[i]->vrtx.data()
    );_glec
  }
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    iboSize,
    indxAll.data(),
    GL_STREAM_DRAW
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
    GL_TRIANGLES, indxCount, GL_UNSIGNED_SHORT, (const GLvoid*)0
  );_glec
  glDisableVertexAttribArray(attr_pos);_glec
  glDisableVertexAttribArray(attr_color);_glec
}


