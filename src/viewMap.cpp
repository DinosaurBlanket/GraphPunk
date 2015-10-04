
#include <iostream>
using std::cout;
using std::endl;
#include <GL/glew.h>

#include "errorsAndLogging.hpp"
#include "glShaderLoader.hpp"
#include "viewMap.hpp"

GLuint VBO;
GLuint IBO;
GLuint shaderProgram;
GLint attr_pos;
GLint attr_color;
GLint unif_transform;
const char *vsPath = "src/mapVrtx.glsl";
const char *fsPath = "src/mapFrag.glsl";

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
float gridUnit;
vec4  gridRect;
mat4  scaledTransform;


void buildOriginSquare() {
  const uint32_t color = 0xff22aa00;
  const uint size = 4;
  float corners[size*2] = {0,1, 1,0, -1,0, 0,-1};
  originSquare.vrtx.resize(size);
  for (uint i = 0; i < size; i++) {
    originSquare.vrtx[i].pos.x = corners[i*2];
    originSquare.vrtx[i].pos.y = corners[i*2 + 1];
    originSquare.vrtx[i].color = color;
  }
  logVrtxData(originSquare.vrtx, 3, "log/originSquareVrtxData.txt");
}

void buildGridLines() {
  const float    halfGridLine  = 1.0/gridUnit;//grid units
  const uint32_t gridLineColor = 0xdd665522;//ABGR
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
  logVrtxData(gridLines.vrtx, 4, "log/gridLinesVrtxData.txt");
}


void initMap(scrollable &scroll, vec2 videoSize) {
  gridUnit = 16;//pixels
  gridRect = vec4(
    -videoSize.x/(gridUnit), videoSize.y/(gridUnit),
    videoSize/vec2(gridUnit/2)
  );//grid units
  
  const float scrollAccel = 1.2;
  scroll.init(
    scrollAccel,
    vec2(gridRect[2]*gridUnit, gridRect[3]*gridUnit),
    -videoSize/vec2(2),
    videoSize
  );
  
  buildOriginSquare();
  buildGridLines();
  
  compileShaders(shaderProgram, vsPath, fsPath);
  unif_transform = glGetUniformLocation(shaderProgram, "transform");_glec
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
  logIndxData(indxAll, 12, "log/indxAll.txt");
  scaledTransform = scale(
    mat4(), vec3((gridUnit*2)/videoSize.x, (gridUnit*2)/videoSize.y, 1.0)
  );
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


void drawMap(scrollable &scroll) {
  vec2 videoSize = scroll.getWinSize();
  mat4 scrolledTransform = translate(
    scaledTransform,
    vec3(
      (scroll.getPos().x + videoSize.x/2)/gridUnit,
      -((scroll.getPos().y + videoSize.y/2)/gridUnit),
      1.0
    )
  );
  glClear(GL_COLOR_BUFFER_BIT);_glec
  glUseProgram(shaderProgram);_glec
  glUniformMatrix4fv(
    unif_transform, 1, GL_FALSE, (const GLfloat*)&scrolledTransform
  );_glec
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



