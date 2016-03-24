
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#include "error.h"
#include "oglTools.h"
#include "../img/uitex.h"
#include "forUi.h"
#include "globalControls.h"


typedef enum {
  pei_iport,
  pei_oport,
  pei_aface,
  pei_mface,
  pei_mhandle
} planeElemId;
typedef enum {dt_n, dt_b, dt_na, dt_ba} dtype;

typedef struct {
  planeElemId  pei;
  uint8_t      iputCount; // ports are in the same planeElem array following this node
  uint8_t      oputCount; // ports are in the same planeElem array following this node
} vinode;

typedef struct {
  planeElemId pei;
  dtype       type;
  uint32_t    node; // offset into plane's planeElems array
  uint8_t     pos;  // 0 is leftmost port, 1 is 2nd leftmost port, etc.
} nodeBase;
typedef struct {nodeBase b;} oport;
typedef struct {
  nodeBase b;
  uint32_t lineVerts; // offset into plane's vert data, both ends of line
  uint32_t oput;      // offset into plane's planeElems
} iport;

typedef union {oport o; iport i; vinode n; planeElemId pei;} planeElem;

typedef struct {
  float      rect_gu[4];
  float      pos_gudc2[2];  // only used when changing planes
  planeElem *planeElems;    // malloced, parallel with vertData
  uint32_t   planeElemCount;
  uint32_t   planeElemCap;
  GLuint     vao;
  GLuint     vbo;
  GLuint     ebo;
  float     *vertData;
  uint32_t  *indxData;
  uint32_t   lineVertsSize; // in elements (floats)
  uint32_t   lineVertsCap;  // in elements (floats)
  uint32_t   nodeVertsSize; // in elements (ints), a rect for each planeElem
  uint32_t   nodeVertsCap;  // in elements (ints), a rect for each planeElem
} plane;
// first verts of every plane are for background
#define backVertsSize 48 // in elements, 12 vertices, 48 floats
#define backElemsSize 36 // in elements (ints)
uint32_t planeVertDataSize(plane *p) {
  return (backVertsSize + p->lineVertsCap + p->nodeVertsCap)*sizeof(float);
}
uint32_t planeElemDataSize(plane *p) {
  return (backElemsSize + 3*((p->lineVertsCap + p->nodeVertsCap)/2))*sizeof(uint32_t);
}
// Module faces are drawn separately
typedef struct {
  plane p;
  //exnode *exnodes;
  // *specialNodes;
} module;

module rootMod = {0};
plane *pln = NULL;
float unitScale_2[2];
const float gridUnit_px = 16;
const float planePadding_gu = 12;

void resetPlaneRect(void) {
  pln->rect_gu[0] = floor(-halfVideoSize_gu2[0] - planePadding_gu); // bl
  pln->rect_gu[1] = floor(-halfVideoSize_gu2[1] - planePadding_gu); // tl
  pln->rect_gu[2] = ceil ( halfVideoSize_gu2[0] + planePadding_gu); // tr
  pln->rect_gu[3] = ceil ( halfVideoSize_gu2[1] + planePadding_gu); // br
  const float backVertData[32] = {
    // inside border
    // bl
    pln->rect_gu[0]+1, pln->rect_gu[1]+1, 
    uitex_ibord_bl_x, uitex_ibord_bl_y,
    // tl
    pln->rect_gu[0]+1, pln->rect_gu[3]-1, 
    uitex_ibord_tl_x, uitex_ibord_tl_y,
    // tr
    pln->rect_gu[2]-1, pln->rect_gu[3]-1, 
    uitex_ibord_tr_x, uitex_ibord_tr_y,
    // br
    pln->rect_gu[2]-1, pln->rect_gu[1]+1, 
    uitex_ibord_br_x, uitex_ibord_br_y,
    
    // outside border
    // bl
    pln->rect_gu[0], pln->rect_gu[1], 
    uitex_obord_bl_x, uitex_obord_bl_y,
    // tl
    pln->rect_gu[0], pln->rect_gu[3], 
    uitex_obord_tl_x, uitex_obord_tl_y,
    // tr
    pln->rect_gu[2], pln->rect_gu[3], 
    uitex_obord_tr_x, uitex_obord_tr_y,
    // br
    pln->rect_gu[2], pln->rect_gu[1], 
    uitex_obord_br_x, uitex_obord_br_y,
  };
  fr(i,32) {pln->vertData[i] = backVertData[i];}
}

void initUiShader(void) {
  uiShader = createShaderProgram(
    "src/vert.glsl", 
    "src/frag.glsl", 
    "uiShader"
  );
  glUseProgram(uiShader);_glec
  unif_unitScale = glGetUniformLocation(uiShader, "unitScale");_glec
  unif_scroll    = glGetUniformLocation(uiShader, "scroll");_glec
  glUniform2f(unif_unitScale, unitScale_2[0], unitScale_2[1]);_glec
  glUniform2f(unif_scroll, 0, 0);_glec
  
  glGenTextures(1, &uiTex);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  texFromBmp(uitex_path);
  glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);_glec
}

void initPlane(void) {
  glGenVertexArrays(1, &pln->vao);_glec
  glBindVertexArray(pln->vao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  
  pln->lineVertsSize =   0;
  pln->nodeVertsSize =   0;
  pln->lineVertsCap  = 120; // arbitrary
  pln->nodeVertsCap  =  60; // arbitrary
  pln->pos_gudc2[0] = 0;
  pln->pos_gudc2[1] = 0;
  
  glGenBuffers(1, &pln->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, pln->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, pln->vbo);_glec
  GLbitfield bufferStorageFlags = 
    GL_MAP_WRITE_BIT      | 
    GL_MAP_PERSISTENT_BIT | 
    GL_MAP_COHERENT_BIT
  ;
  uint32_t bufSize = planeVertDataSize(pln);
  glBufferStorage(
    GL_ARRAY_BUFFER,    // GLenum        target
    bufSize,            // GLsizeiptr    size​
    0,                  // const GLvoid *data​
    bufferStorageFlags  // GLbitfield    flags​
  );_glec
  pln->vertData = glMapBufferRange(
    GL_ARRAY_BUFFER,    // GLenum     target​
    0,                  // GLintptr   offset​
    bufSize,            // GLsizeiptr length​
    bufferStorageFlags  // GLbitfield access
  );_glec
  
  glGenBuffers(1, &pln->ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pln->ebo);_glec
  bufSize = planeElemDataSize(pln);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, bufSize, 0, bufferStorageFlags);_glec
  pln->indxData = glMapBufferRange(
    GL_ELEMENT_ARRAY_BUFFER, 0, bufSize, bufferStorageFlags
  );_glec
  
  // the first 8 verts are set by this function
  resetPlaneRect();
  // the next 4 are set below
  float centerVerts[16];
  const float centerVertsRect_gu[4] = {-1.0, -1.0, 1.0, 1.0};
  const float centerTexRect_nt[4] = {
    uitex_cntr_bl_x, uitex_cntr_bl_y, uitex_cntr_tr_x, uitex_cntr_tr_y
  };
  mapTexRectToVerts(centerVerts, centerVertsRect_gu, centerTexRect_nt);
  for (int i = backVertsSize-16; i < backVertsSize; i++) {
    pln->vertData[i] = centerVerts[i-(backVertsSize-16)];
  }
  
  // indx data
  const uint32_t backElems[backElemsSize] = {
    // inside border
    0,1,3, 1,2,3,
    // outside border
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
    // center marker
    8,9,11, 9,10,11
  };
  fr(i,backElemsSize) {pln->indxData[i] = backElems[i];}
  setRectElems(&pln->indxData[backElemsSize], bufSize/sizeof(float));
  
  setUiVertAttribs();
}

#include <stdio.h>
void initRoot(float videoSize_px2[2]) {
  fr(i,2) {unitScale_2[i] = gridUnit_px/(videoSize_px2[i]/2.0f);}
  fr(i,2) {halfVideoSize_gu2[i] = (videoSize_px2[i]/gridUnit_px)/2.0f;}
  pln = &rootMod.p;
  initUiShader();
  initPlane();
  initGc();
  
  
  printf(
    "sizeof(nodeBase) :%3li\n"
    "sizeof(oport)    :%3li\n"
    "sizeof(iport)    :%3li\n"
    "sizeof(vinode)   :%3li\n"
    "sizeof(planeElem):%3li\n"
    "sizeof(plane)    :%3li\n",
    sizeof(nodeBase),
    sizeof(oport),
    sizeof(iport),
    sizeof(vinode),
    sizeof(planeElem),
    sizeof(plane)
  );
  
  
}

void posPxToPosGu(float pos_gu[2], const int posX_px, const int posY_px) {
  pos_gu[0] =  posX_px/gridUnit_px - halfVideoSize_gu2[0];
  pos_gu[1] = -posY_px/gridUnit_px + halfVideoSize_gu2[1];
}

float newCurs_gu3[3]       = {0}; // cursor state relative to screen
float oldCurs_gu3[3]       = {0};
float clickDnCurs_gu3[3]   = {0};
float newScroll_gu2[2]     = {0}; // plane center to screen center difference
float oldScroll_gu2[2]     = {0};
float clickDnScroll_gu2[2] = {0};
float scrollVel_gu2[2]     = {0};

void onDragScroll(void *data) {
  fr(i,2) {
    newScroll_gu2[i] = clickDnScroll_gu2[i]-(clickDnCurs_gu3[i]-newCurs_gu3[i]);
  }
}
void onClickUpScroll(void *data) {
  fr(i,2) {scrollVel_gu2[i] = newCurs_gu3[i] - oldCurs_gu3[i];}
}

void clickDn(int posX_px, int posY_px) {
  posPxToPosGu(newCurs_gu3, posX_px, posY_px);
  newCurs_gu3[2] = 1.0f;
  fr(i,3) {clickDnCurs_gu3[i] = newCurs_gu3[i];}
  fr(i,2) {clickDnScroll_gu2[i] = newScroll_gu2[i];}
  if (!onClickGc(newCurs_gu3)) {
    fr(i,2) {scrollVel_gu2[i] = 0;}
    onDrag    = onDragScroll;
    onClickUp = onClickUpScroll;
  }
}
void curMove(int posX_px, int posY_px) {
  posPxToPosGu(newCurs_gu3, posX_px, posY_px);
  if (newCurs_gu3[2]) onDrag(NULL);
}
void clickUp(int posX_px, int posY_px) {
  posPxToPosGu(newCurs_gu3, posX_px, posY_px);
  newCurs_gu3[2] = 0;
  onClickUp(NULL);
}

float screenCrnrs_gu4[4]  = {0}; // xyxy, bl tr, relative to plane center
bool redrawPlane = true;

void perFrame(void) {
  fr(i,2) {newScroll_gu2[i] += scrollVel_gu2[i];}
  if (!allEq(newScroll_gu2, oldScroll_gu2, 2)) {
    screenCrnrs_gu4[0] = newScroll_gu2[0]-halfVideoSize_gu2[0];
    screenCrnrs_gu4[1] = newScroll_gu2[1]-halfVideoSize_gu2[1];
    screenCrnrs_gu4[2] = newScroll_gu2[0]+halfVideoSize_gu2[0];
    screenCrnrs_gu4[3] = newScroll_gu2[1]+halfVideoSize_gu2[1];
    fr(i,2) {
      if (screenCrnrs_gu4[i] < pln->rect_gu[i]) {
        newScroll_gu2[i] = pln->rect_gu[i]+halfVideoSize_gu2[i];
        scrollVel_gu2[i] = 0;
      }
      else if (screenCrnrs_gu4[i+2] > pln->rect_gu[i+2]) {
        newScroll_gu2[i] = pln->rect_gu[i+2]-halfVideoSize_gu2[i];
        scrollVel_gu2[i] = 0;
      }
    }
    redrawPlane = true;
  }
  if (redrawPlane || redrawGc) {
    if (redrawPlane) {
      glBindVertexArray(pln->vao);_glec
      glUniform2f(unif_scroll, newScroll_gu2[0], newScroll_gu2[1]);_glec
      glDrawElements(GL_TRIANGLES, backElemsSize, GL_UNSIGNED_INT, 0);_glec
      redrawPlane = false;
    }
    drawGc();
  }
  fr(i,3) {oldCurs_gu3[i] = newCurs_gu3[i];}
  fr(i,2) {oldScroll_gu2[i] = newScroll_gu2[i];}
  glFinish();
}
