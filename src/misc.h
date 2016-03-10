#pragma once

#include <stdbool.h>

#define fr(i, bound) for (int i = 0; i < (bound); i++)
bool allEq(const float *l, const float *r, int c);

typedef struct {float x; float y; float s; float t;} uiVert;
typedef struct {
  GLuint    vbo;
  uint32_t  vCount; // number of vertex elements
  uint32_t  vCap;   // maximum number of elements the buffer can hold
  GLuint    ebo;
  uint32_t  eCount;
  uint32_t  eCap;
} vertGroup;
void drawVertGroup(vertGroup *vg);


typedef struct {
  float       corners_gu4[4]; // xyxy, bottom left and top right
  float       pos_gudc2[2];
  vertGroup   vg;
  //uiVert     *lineVertData
  //uiVert     *nodeVertData
  //int       depth; // in module tree
  //vinode *vinodes;
} plane;

typedef struct {
  plane p;
  //exnode *exnodes;
  // *specialNodes;
} module;

void resetPlaneCorners(plane *pln, float halfVideoSize_gu2[2]);
void initPlane(plane *pln, float halfVideoSize_gu2[2]);

#define GlorolsButCount 11
void initGlorolsVerts(vertGroup *vg, float halfVideoSize_gu2[2]);
