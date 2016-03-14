
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#include "error.h"
#include "oglTools.h"
#include "../img/uitex.h"

#define fr(i, bound) for (int i = 0; i < (bound); i++)

typedef struct {float x; float y; float s; float t;} uiVert;

typedef struct {
  float    corners_gu4[4]; // xyxy, bottom left and top right
  float    pos_gudc2[2];
  GLuint   vao;
  GLuint   vbo;
  GLuint   ebo;
  uint32_t lineCount;
  uint32_t lineCap;
  uint32_t nodeCount;
  uint32_t nodeCap;
  //int    depth; // in module tree
  //vinode *vinodes; // vinodes keep track of their data location in vao
} plane;

typedef struct {
  plane p;
  //exnode *exnodes;
  // *specialNodes;
} module;

bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}

void printVerts(const uiVert *verts, int count) {
  fr(i,count) {
    printf(
      "%2i: vert pos, gu: %4.2f, %4.2f  -  tex pos, nt: %6.5f, %6.5f\n",
      i, verts[i].x, verts[i].y, verts[i].s, verts[i].t
    );
  }
}

void mapTexRectToVerts(
  uiVert     *destVerts,
  const float destRect_gu[4], // grid units
  const float srcRect_nt[4]   // normalized texture coordinates
) {
  // bl
  destVerts[0].x = destRect_gu[0];
  destVerts[0].y = destRect_gu[1];
  destVerts[0].s = srcRect_nt[0];
  destVerts[0].t = srcRect_nt[1];
  // tl
  destVerts[1].x = destRect_gu[0];
  destVerts[1].y = destRect_gu[3];
  destVerts[1].s = srcRect_nt[0];
  destVerts[1].t = srcRect_nt[3];
  // tr
  destVerts[2].x = destRect_gu[2];
  destVerts[2].y = destRect_gu[3];
  destVerts[2].s = srcRect_nt[2];
  destVerts[2].t = srcRect_nt[3];
  // br
  destVerts[3].x = destRect_gu[2];
  destVerts[3].y = destRect_gu[1];
  destVerts[3].s = srcRect_nt[2];
  destVerts[3].t = srcRect_nt[1];
}

module rootMod = {0};
plane *pln = NULL;
float unitScale_2[2];
float halfVideoSize_gu2[2];
const float gridUnit_px = 16;
const float planePadding_gu = 12;

void resetPlaneRect() {
  pln->corners_gu4[0] = floor(-halfVideoSize_gu2[0] - planePadding_gu); // bl
  pln->corners_gu4[1] = floor(-halfVideoSize_gu2[1] - planePadding_gu); // tl
  pln->corners_gu4[2] = ceil ( halfVideoSize_gu2[0] + planePadding_gu); // tr
  pln->corners_gu4[3] = ceil ( halfVideoSize_gu2[1] + planePadding_gu); // br
  uiVert backVerts[8] = {
    // inside border
    {    // 0 bl
      pln->corners_gu4[0]+1, pln->corners_gu4[1]+1, 
      uitex_ibord_bl_x, uitex_ibord_bl_y
    }, { // 1 tl
      pln->corners_gu4[0]+1, pln->corners_gu4[3]-1, 
      uitex_ibord_tl_x, uitex_ibord_tl_y
    }, { // 2 tr
      pln->corners_gu4[2]-1, pln->corners_gu4[3]-1, 
      uitex_ibord_tr_x, uitex_ibord_tr_y
    }, { // 3 br
      pln->corners_gu4[2]-1, pln->corners_gu4[1]+1, 
      uitex_ibord_br_x, uitex_ibord_br_y
    }, 
    // outside border
    {    // 0 bl
      pln->corners_gu4[0], pln->corners_gu4[1], 
      uitex_obord_bl_x, uitex_obord_bl_y
    }, { // 1 tl
      pln->corners_gu4[0], pln->corners_gu4[3], 
      uitex_obord_tl_x, uitex_obord_tl_y
    }, { // 2 tr
      pln->corners_gu4[2], pln->corners_gu4[3], 
      uitex_obord_tr_x, uitex_obord_tr_y
    }, { // 3 br
      pln->corners_gu4[2], pln->corners_gu4[1], 
      uitex_obord_br_x, uitex_obord_br_y
    }, 
  };
  
  glBindVertexArray(pln->vao);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backVerts), backVerts);_glec
}

GLuint uiShader;
GLint  unif_scroll;
GLint  unif_unitScale;
GLuint uiTex;
void initUiShader() {
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

void setUiVertAttribs() {
  GLint attr_pos      = glGetAttribLocation(uiShader, "pos");_glec
  GLint attr_texCoord = glGetAttribLocation(uiShader, "texCoord");_glec
  glEnableVertexAttribArray(attr_pos  );_glec
  glEnableVertexAttribArray(attr_texCoord);_glec
  glVertexAttribPointer(
    attr_pos,      2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)0
  );_glec
  glVertexAttribPointer(
    attr_texCoord, 2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)8
  );_glec
}

// first verts of every plane are for background
#define lineVertOffset 12
#define lineElemOffset 36

void initPlane() {
  glGenVertexArrays(1, &pln->vao);_glec
  glBindVertexArray(pln->vao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  
  glGenBuffers(1, &pln->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER,         pln->vbo);_glec
  glBufferData(
    GL_ARRAY_BUFFER,               // GLenum        target
    lineVertOffset*sizeof(uiVert), // GLsizeiptr    size
    0,                             // const GLvoid *data
    GL_STATIC_DRAW                 // GLenum        usage​
  );_glec
  // the first 8 verts are set by this function
  resetPlaneRect(pln, halfVideoSize_gu2);
  // the next 4 are set below
  uiVert centerVerts[4];
  const float centerVertsRect_gu[4] = {-1.0, -1.0, 1.0, 1.0};
  const float centerTexRect_nt[4] = {
    uitex_cntr_bl_x, uitex_cntr_bl_y, uitex_cntr_tr_x, uitex_cntr_tr_y
  };
  mapTexRectToVerts(centerVerts, centerVertsRect_gu, centerTexRect_nt);
  glBufferSubData(
    GL_ARRAY_BUFFER,
    8*sizeof(uiVert),
    sizeof(centerVerts),
    centerVerts
  );_glec
  
  glGenBuffers(1, &pln->ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pln->ebo);_glec
  uint32_t backInd[] = {
    // inside border
    0,1,3, 1,2,3,
    // outside border
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
    // center marker
    8,9,11, 9,10,11
  };
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(backInd),
    backInd,
    GL_STATIC_DRAW
  );_glec
  
  setUiVertAttribs(uiShader);
}

void setRectElems(uint32_t *elems, const uint32_t rectCount) {
  uint32_t v = 0;
  uint32_t e = 0;
  for (; v < 4*rectCount; v += 4, e += 6) {
    elems[e  ] = v;
    elems[e+1] = v+1;
    elems[e+2] = v+3;
    elems[e+3] = v+1;
    elems[e+4] = v+2;
    elems[e+5] = v+3;
  }
}

GLuint  glorolsVao;
#define glorolsButCount 11
#define butSide_gu 2.0f
float   golorolsRect_gu[4];

void initGlorols() {
  glGenVertexArrays(1, &glorolsVao);_glec
  glBindVertexArray(glorolsVao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  
  golorolsRect_gu[0] = -butSide_gu*(glorolsButCount/2.0f);
  golorolsRect_gu[1] = halfVideoSize_gu2[1] - butSide_gu;
  golorolsRect_gu[2] =  butSide_gu*(glorolsButCount/2.0f);
  golorolsRect_gu[3] = halfVideoSize_gu2[1];
  
  const int vertCount = 4*glorolsButCount;
  const int elemCount = 6*glorolsButCount;
  const float uitexButRect[2*glorolsButCount] = {
    uitex_gc_play_bl_x,     uitex_gc_play_bl_y,
    uitex_gc_step_bl_x,     uitex_gc_step_bl_y,
    uitex_gc_unmuted_bl_x,  uitex_gc_unmuted_bl_y,
    uitex_gc_unsoloed_bl_x, uitex_gc_unsoloed_bl_y,
    uitex_gc_moveNode_bl_x, uitex_gc_moveNode_bl_y,
    uitex_gc_unLock_bl_x,   uitex_gc_unLock_bl_y,
    uitex_gc_up_bl_x,       uitex_gc_up_bl_y,
    uitex_gc_top_bl_x,      uitex_gc_top_bl_y,
    uitex_gc_back_bl_x,     uitex_gc_back_bl_y,
    uitex_gc_forward_bl_x,  uitex_gc_forward_bl_y,
    uitex_gc_save_bl_x,     uitex_gc_save_bl_y
  };
  float vertRect[4] = {
    golorolsRect_gu[0],              golorolsRect_gu[1], 
    golorolsRect_gu[0] + butSide_gu, golorolsRect_gu[3]
  };
  uiVert verts[vertCount];
  float texRect[4];
  fr(b, glorolsButCount) {
    texRect[0] = uitexButRect[2*b];
    texRect[1] = uitexButRect[2*b + 1];
    texRect[2] = uitexButRect[2*b]     + uitex_gc_buttonSide;
    texRect[3] = uitexButRect[2*b + 1] + uitex_gc_buttonSide;
    mapTexRectToVerts(&verts[4*b], vertRect, texRect);
    vertRect[0] += butSide_gu;
    vertRect[2] += butSide_gu;
  }
  GLuint vbo;
  glGenBuffers(1, &vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, vbo);_glec
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);_glec
  
  uint32_t elems[elemCount];
  setRectElems(elems, glorolsButCount);
  GLuint ebo;
  glGenBuffers(1, &ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);_glec
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW
  );_glec
  
  setUiVertAttribs(uiShader);
}


void initRoot(float videoSize_px2[2]) {
  fr(i,2) {unitScale_2[i] = gridUnit_px/(videoSize_px2[i]/2.0f);}
  fr(i,2) {halfVideoSize_gu2[i] = (videoSize_px2[i]/gridUnit_px)/2.0f;}
  pln = &rootMod.p;
  initUiShader();
  initPlane();
  initGlorols();
}

bool pointIsInRect(const float point[2], const float rect[4]) {
  return 
    point[0] > rect[0] &&
    point[0] < rect[2] &&
    point[1] > rect[1] &&
    point[1] < rect[3]
  ;
}

void posPxToPosGu(float pos_gu[2], const int posX_px, const int posY_px) {
  pos_gu[0] =  posX_px/gridUnit_px - halfVideoSize_gu2[0];
  pos_gu[1] = -posY_px/gridUnit_px + halfVideoSize_gu2[1];
}


float newCursAbs_gu3[3]   = {0}; // cursor state relative to screen
float oldCursAbs_gu3[3]   = {0};
float newScrollPos_gu2[2] = {0}; // plane center to screen center difference
float oldScrollPos_gu2[2] = {0};
float scrollVel_gu2[2]    = {0};

typedef void (*cursEvenHandler)(void);
void onDragScroll(void) {
  fr(i,2) {newScrollPos_gu2[i] += newCursAbs_gu3[i] - oldCursAbs_gu3[i];}
}
void onClickDnScroll(void) {
  fr(i,2) {scrollVel_gu2[i] = 0;}
}
void onClickUpScroll(void) {
  fr(i,2) {scrollVel_gu2[i] = newCursAbs_gu3[i] - oldCursAbs_gu3[i];}
}
void doNothing(void) {}

cursEvenHandler onClickDn = NULL;
cursEvenHandler onDrag    = NULL;
cursEvenHandler onClickUp = NULL;

void clickDn(int posX_px, int posY_px) {
  posPxToPosGu(newCursAbs_gu3, posX_px, posY_px);
  newCursAbs_gu3[2] = 1.0f;
  if (pointIsInRect(newCursAbs_gu3, golorolsRect_gu)) {
    onDrag    = doNothing;
    onClickDn = doNothing;
    onClickUp = doNothing;
  }
  else {
    onDrag    = onDragScroll;
    onClickDn = onClickDnScroll;
    onClickUp = onClickUpScroll;
  }
  onClickDn();
}
void curMove(int posX_px, int posY_px) {
  posPxToPosGu(newCursAbs_gu3, posX_px, posY_px);
  if (newCursAbs_gu3[2]) onDrag();
}
void clickUp(int posX_px, int posY_px) {
  posPxToPosGu(newCursAbs_gu3, posX_px, posY_px);
  newCursAbs_gu3[2] = 0;
  onClickUp();
}


float screenCrnrs_gu4[4]  = {0}; // xyxy, bl tr, relative to plane center
bool redrawPlane   = true;
bool redrawGlorols = true;

void perFrame() {
  fr(i,2) {newScrollPos_gu2[i] += scrollVel_gu2[i];}
  if (!allEq(newScrollPos_gu2, oldScrollPos_gu2, 2)) {
    screenCrnrs_gu4[0] = newScrollPos_gu2[0]-halfVideoSize_gu2[0];
    screenCrnrs_gu4[1] = newScrollPos_gu2[1]-halfVideoSize_gu2[1];
    screenCrnrs_gu4[2] = newScrollPos_gu2[0]+halfVideoSize_gu2[0];
    screenCrnrs_gu4[3] = newScrollPos_gu2[1]+halfVideoSize_gu2[1];
    fr(i,2) {
      if (screenCrnrs_gu4[i] < pln->corners_gu4[i]) {
        newScrollPos_gu2[i] = pln->corners_gu4[i]+halfVideoSize_gu2[i];
        scrollVel_gu2[i] = 0;
      }
      else if (screenCrnrs_gu4[i+2] > pln->corners_gu4[i+2]) {
        newScrollPos_gu2[i] = pln->corners_gu4[i+2]-halfVideoSize_gu2[i];
        scrollVel_gu2[i] = 0;
      }
    }
    redrawPlane = true;
  }
  if (redrawPlane || redrawGlorols) {
    if (redrawPlane) {
      glBindVertexArray(pln->vao);_glec
      glUniform2f(unif_scroll, newScrollPos_gu2[0], newScrollPos_gu2[1]);_glec
      glDrawElements(GL_TRIANGLES, lineElemOffset, GL_UNSIGNED_INT, 0);_glec
      redrawPlane = false;
    }
    glBindVertexArray(glorolsVao);_glec
    glUniform2f(unif_scroll, 0, 0);_glec
    glDrawElements(GL_TRIANGLES, 6*glorolsButCount, GL_UNSIGNED_INT, 0);_glec
    redrawGlorols = false;
  }
  fr(i,3) {oldCursAbs_gu3[i] = newCursAbs_gu3[i];}
  fr(i,2) {oldScrollPos_gu2[i] = newScrollPos_gu2[i];}
}
