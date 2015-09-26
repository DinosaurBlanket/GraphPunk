
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
  vec2     pos;
  uint32_t color;
};
typedef uint16_t indx_t ;
struct vrtxGroup {
  std::vector<vertex> vrtx;
  uint32_t vboStart;
};
vrtxGroup gridLines;
vrtxGroup originSquare;
std::vector<vrtxGroup*> vrtxGroups = {&originSquare, &gridLines};
std::vector<indx_t> indxAll;

uint vrtxCount = 0;
uint indxCount = 0;
mat4 transform;

#if LOGGING_ENABLED
#include <iostream>
#include <fstream>
void logVrtxData(
  std::vector<vertex> &vrtxData,
  uint pitch,
  const char *path
) {
  std::ofstream of;
  of.open(path);
  for (uint32_t i = 0; i < vrtxData.size(); i ++) {
    of <<
    vrtxData[i].pos.x << "\t" <<
    vrtxData[i].pos.y << "\t" <<
    vrtxData[i].pos.z << "\t\t";
    if (i % pitch == pitch-1) of << endl;
  }
  of.close();
}
void logIndxData(
  std::vector<indx_t> &indxData,
  uint pitch,
  const char *path
) {
  std::ofstream of;
  of.open(path);
  for (uint32_t i = 0; i < indxData.size(); i ++) {
    of << indxData[i] << '\t';
    if (i % pitch == pitch-1) of << endl;
  }
  of.close();
}
#endif


void view_map_init(vec4 gridRect, vec2 screenSize, float gridUnit) {
  {//originSquare
    const uint32_t color = 0xff114400;
    const uint size = 4;
    float corners[size*2] = {-1, 1,   1, 1,  -1,-1,   1,-1};
    originSquare.vrtx.resize(size);
    for (uint i = 0; i < size; i++) {
      originSquare.vrtx[i].pos.x = corners[i*2];
      originSquare.vrtx[i].pos.y = corners[i*2 + 1];
      originSquare.vrtx[i].color = color;
    }
    #if LOGGING_ENABLED
    logVrtxData(originSquare.vrtx, 3, "log/originSquareVrtxData.txt");
    #endif
  }
  {//gridLines
    const float    halfGridLine  = 1.0/gridUnit;//grid units
    const uint32_t gridLineColor = 0xff886622;//ABGR
    const int hrzlGridLineVrtxCount = (gridRect[2] + 1) * 4;
    const int vrtlGridLineVrtxCount = (gridRect[3] + 1) * 4;
    const int gridLineVrtxCount = hrzlGridLineVrtxCount + vrtlGridLineVrtxCount;
    gridLines.vrtx.resize(gridLineVrtxCount);
    int i = 0; float x = 0, y = 0;
    for (; i < hrzlGridLineVrtxCount; i += 4, x++) {
      gridLines.vrtx[i  ].color = gridLineColor;
      gridLines.vrtx[i  ].pos = vec2(
        x+gridRect[0]-halfGridLine, gridRect[1]
      );
      gridLines.vrtx[i+1].color = gridLineColor;
      gridLines.vrtx[i+1].pos = vec2(
        x+gridRect[0]+halfGridLine, gridRect[1]
      );
      gridLines.vrtx[i+2].color = gridLineColor;
      gridLines.vrtx[i+2].pos = vec2(
        x+gridRect[0]-halfGridLine, gridRect[1]-gridRect[3]
      );
      gridLines.vrtx[i+3].color = gridLineColor;
      gridLines.vrtx[i+3].pos = vec2(
        x+gridRect[0]+halfGridLine, gridRect[1]-gridRect[3]
      );
    }
    for (i = hrzlGridLineVrtxCount, y = 0; i < gridLineVrtxCount; i += 4, y++) {
      gridLines.vrtx[i  ].color = gridLineColor;
      gridLines.vrtx[i  ].pos = vec2(
        gridRect[0],             (y-gridRect[1])-halfGridLine
      );
      gridLines.vrtx[i+1].color = gridLineColor;
      gridLines.vrtx[i+1].pos = vec2(
        gridRect[0]+gridRect[2], (y-gridRect[1])-halfGridLine
      );
      gridLines.vrtx[i+2].color = gridLineColor;
      gridLines.vrtx[i+2].pos = vec2(
        gridRect[0],             (y-gridRect[1])+halfGridLine
      );
      gridLines.vrtx[i+3].color = gridLineColor;
      gridLines.vrtx[i+3].pos = vec2(
        gridRect[0]+gridRect[2], (y-gridRect[1])+halfGridLine
      );
    }
    #if LOGGING_ENABLED
    logVrtxData(gridLines.vrtx, 4, "log/gridLinesVrtxData.txt");
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
  for (uint i = 0; i < vrtxGroups.size(); i++) {
    vrtxCount += vrtxGroups[i]->vrtx.size();
    vrtxGroups[i]->vboStart =
      !i ? 0 : vrtxGroups[i-1]->vboStart + vrtxGroups[i-1]->vrtx.size();
  }
  indxCount = vrtxCount * 1.5;
  indxAll.resize(indxCount);
  uint i = 0, v = 0;
  for (; i < indxCount; i += 6, v += 4) {
    indxAll[i  ] = v;
    indxAll[i+1] = v + 1;
    indxAll[i+2] = v + 2;
    indxAll[i+3] = v + 1;
    indxAll[i+4] = v + 2;
    indxAll[i+5] = v + 3;
  }
  #if LOGGING_ENABLED
  logIndxData(indxAll, 12, "log/indxAll.txt");
  #endif
  const int vboSize = sizeof(vertex)*vrtxCount;
  const int iboSize = sizeof(indx_t)*indxCount;
  glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_STREAM_DRAW);_glec
  for (uint i = 0; i < vrtxGroups.size(); i++) {
    glBufferSubData(
      GL_ARRAY_BUFFER,
      sizeof(vertex)*vrtxGroups[i]->vboStart,
      sizeof(vertex)*vrtxGroups[i]->vrtx.size(),
      (const GLvoid*)vrtxGroups[i]->vrtx.data()
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
    attr_pos,   2, GL_FLOAT,         false, 12, (const GLvoid*)0
  );_glec
  glVertexAttribPointer(
    attr_color, 4, GL_UNSIGNED_BYTE, true,  12, (const GLvoid*)8
  );_glec
  glDrawElements(
    GL_TRIANGLES, indxCount, GL_UNSIGNED_SHORT, (const GLvoid*)0
  );_glec
  glDisableVertexAttribArray(attr_pos);_glec
  glDisableVertexAttribArray(attr_color);_glec
}

void displayGCB() {
  glClear(GL_COLOR_BUFFER_BIT);_glec
  view_map_draw();
  glutSwapBuffers();
}
