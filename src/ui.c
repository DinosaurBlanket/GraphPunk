
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

typedef struct {
  float    rect_gu[4];
  //float    pos_gudc2[2]; // "newScroll_gu2"
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

void printVerts(const float *vertData, int vertCount) {
  fr(i,vertCount) {
    printf(
      "%2i: vert pos, gu: %4.2f, %4.2f  -  tex pos, nt: %6.5f, %6.5f\n",
      i, vertData[4*i], vertData[4*i + 1], vertData[4*i + 2], vertData[4*i + 3]
    );
  }
}

void mapTexRectToVerts(
  float      *destVertData,
  const float destRect_gu[4], // grid units
  const float srcRect_nt[4]   // normalized texture coordinates
) {
  // bl
  destVertData[ 0] = destRect_gu[0];
  destVertData[ 1] = destRect_gu[1];
  destVertData[ 2] =  srcRect_nt[0];
  destVertData[ 3] =  srcRect_nt[1];
  // tl
  destVertData[ 4] = destRect_gu[0];
  destVertData[ 5] = destRect_gu[3];
  destVertData[ 6] =  srcRect_nt[0];
  destVertData[ 7] =  srcRect_nt[3];
  // tr
  destVertData[ 8] = destRect_gu[2];
  destVertData[ 9] = destRect_gu[3];
  destVertData[10] =  srcRect_nt[2];
  destVertData[11] =  srcRect_nt[3];
  // br
  destVertData[12] = destRect_gu[2];
  destVertData[13] = destRect_gu[1];
  destVertData[14] =  srcRect_nt[2];
  destVertData[15] =  srcRect_nt[1];
}

module rootMod = {0};
plane *pln = NULL;
float unitScale_2[2];
float halfVideoSize_gu2[2];
const float gridUnit_px = 16;
const float planePadding_gu = 12;

void resetPlaneRect() {
  pln->rect_gu[0] = floor(-halfVideoSize_gu2[0] - planePadding_gu); // bl
  pln->rect_gu[1] = floor(-halfVideoSize_gu2[1] - planePadding_gu); // tl
  pln->rect_gu[2] = ceil ( halfVideoSize_gu2[0] + planePadding_gu); // tr
  pln->rect_gu[3] = ceil ( halfVideoSize_gu2[1] + planePadding_gu); // br
  float backVertData[32] = {
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
  
  glBindVertexArray(pln->vao);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backVertData), backVertData);_glec
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
  glEnableVertexAttribArray(attr_pos);_glec
  glEnableVertexAttribArray(attr_texCoord);_glec
  glVertexAttribPointer(
    attr_pos,      2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)0
  );_glec
  glVertexAttribPointer(
    attr_texCoord, 2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)8
  );_glec
}

// first verts of every plane are for background
#define lineVertOffset 48 // 12 vertices, 48 floats
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
    lineVertOffset*sizeof(float),  // GLsizeiptr    size
    0,                             // const GLvoid *data
    GL_STATIC_DRAW                 // GLenum        usage​
  );_glec
  // the first 8 verts are set by this function
  resetPlaneRect(pln, halfVideoSize_gu2);
  // the next 4 are set below
  float centerVerts[16];
  const float centerVertsRect_gu[4] = {-1.0, -1.0, 1.0, 1.0};
  const float centerTexRect_nt[4] = {
    uitex_cntr_bl_x, uitex_cntr_bl_y, uitex_cntr_tr_x, uitex_cntr_tr_y
  };
  mapTexRectToVerts(centerVerts, centerVertsRect_gu, centerTexRect_nt);
  glBufferSubData(
    GL_ARRAY_BUFFER,
    32*sizeof(float),
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

typedef void (*cursEventHandler)(void *data);

cursEventHandler onDrag    = NULL;
cursEventHandler onClickUp = NULL;

void doNothing(void *data) {}

typedef struct {
  float           *verts; // will likely be part of a larger array
  cursEventHandler onClickDn;
  cursEventHandler onDrag;
  cursEventHandler onClickUp;
} uiElement;


GLuint gc_vao;
GLuint gc_vbo;
GLuint gc_ebo;
const float gc_butSide_gu = 2;
float  gc_rect_gu[4];
typedef enum {
  gcid_play, gcid_step, gcid_mute, gcid_solo, gcid_mvBr, gcid_lock,
  gcid_up,   gcid_top,  gcid_back, gcid_frwd, gcid_save, gcid_count
} gcid;
const float uitexButCorners_nt[2*gcid_count] = {
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
#define   gc_vertCount (4*gcid_count)  // 4 vertices per button
float     gc_vertData[4*gc_vertCount]; // 4 floats per vertex, 16 per button
uiElement gc_uiElems[gcid_count];

bool gc_paused;
bool gc_muted;
bool gc_soloed;
bool gc_moveBranch;
bool gc_locked;
bool redrawPlane = true;
bool redrawGc    = true;

void gc_onPlayPauseDn(void *data) {
  puts("in gc_onPlayPauseDn");
  if (gc_paused) {
    gc_paused = false;
    // x  y  s  T  x  y  s  T  x  y  s  T  x  y  s  T
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    gc_vertData[16*gcid_play +  3] = uitex_gc_play_bl_y;
    gc_vertData[16*gcid_play +  7] = uitex_gc_play_bl_y + uitex_gc_buttonSide;
    gc_vertData[16*gcid_play + 11] = uitex_gc_play_bl_y + uitex_gc_buttonSide;
    gc_vertData[16*gcid_play + 15] = uitex_gc_play_bl_y;
  }
  else {
    gc_paused = true;
    gc_vertData[16*gcid_play +  3] = uitex_gc_play_bl_y +   uitex_gc_buttonSide;
    gc_vertData[16*gcid_play +  7] = uitex_gc_play_bl_y + 2*uitex_gc_buttonSide;
    gc_vertData[16*gcid_play + 11] = uitex_gc_play_bl_y + 2*uitex_gc_buttonSide;
    gc_vertData[16*gcid_play + 15] = uitex_gc_play_bl_y +   uitex_gc_buttonSide;
  }
  glBindVertexArray(gc_vao);_glec
  glBufferSubData(
    GL_ARRAY_BUFFER, 16*gcid_play*sizeof(float), 16*sizeof(float), gc_vertData
  );_glec
  redrawGc = true;
}


void initGc() {
  glGenVertexArrays(1, &gc_vao);_glec
  glBindVertexArray(gc_vao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  
  gc_rect_gu[0] = -gc_butSide_gu*(gcid_count/2.0f);
  gc_rect_gu[1] = halfVideoSize_gu2[1] - gc_butSide_gu;
  gc_rect_gu[2] =  gc_butSide_gu*(gcid_count/2.0f);
  gc_rect_gu[3] = halfVideoSize_gu2[1];
  
  float butRect[4];
  butRect[1] = gc_rect_gu[1];
  butRect[3] = gc_rect_gu[3];
  const int elemCount = 6*gcid_count;
  float texRect[4];
  fr(b, gcid_count) {
    butRect[0] = gc_rect_gu[0] + gc_butSide_gu*b;
    butRect[2] = gc_rect_gu[0] + gc_butSide_gu*(b+1);
    texRect[0] = uitexButCorners_nt[2*b    ];
    texRect[1] = uitexButCorners_nt[2*b + 1];
    texRect[2] = uitexButCorners_nt[2*b    ] + uitex_gc_buttonSide;
    texRect[3] = uitexButCorners_nt[2*b + 1] + uitex_gc_buttonSide;
    mapTexRectToVerts(&gc_vertData[16*b], butRect, texRect);
    gc_uiElems[b].verts     = &gc_vertData[16*b]; // wrong
    gc_uiElems[b].onClickDn = doNothing;
    gc_uiElems[b].onDrag    = doNothing;
    gc_uiElems[b].onClickUp = doNothing;
  }
  gc_uiElems[0].onClickDn = gc_onPlayPauseDn;
  
  
  glGenBuffers(1, &gc_vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, gc_vbo);_glec
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(gc_vertData), gc_vertData, GL_STATIC_DRAW
  );_glec
  
  uint32_t elems[elemCount];
  setRectElems(elems, gcid_count);
  glGenBuffers(1, &gc_ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gc_ebo);_glec
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
  initGc();
}

bool pointIsInRect(const float point[2], const float rect[4]) {
  return 
    point[0] > rect[0] &&
    point[0] < rect[2] &&
    point[1] > rect[1] &&
    point[1] < rect[3]
  ;
}
bool pointIsInVertRect(const float point[2], const float vertRect[16]) {
  return 
    point[0] > vertRect[0] &&
    point[0] < vertRect[8] &&
    point[1] > vertRect[1] &&
    point[1] < vertRect[9]
  ;
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
  if (pointIsInRect(newCurs_gu3, gc_rect_gu)) {
    fr(b,gcid_count) {
      if (pointIsInVertRect(newCurs_gu3, gc_uiElems[b].verts)) {
        onDrag    = gc_uiElems[b].onDrag;
        onClickUp = gc_uiElems[b].onClickUp;
        gc_uiElems[b].onClickDn(NULL);
        break;
      }
    }
  }
  else {
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

void perFrame() {
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
      glDrawElements(GL_TRIANGLES, lineElemOffset, GL_UNSIGNED_INT, 0);_glec
      redrawPlane = false;
    }
    glBindVertexArray(gc_vao);_glec
    glUniform2f(unif_scroll, 0, 0);_glec
    glDrawElements(GL_TRIANGLES, 6*gcid_count, GL_UNSIGNED_INT, 0);_glec
    redrawGc = false;
  }
  fr(i,3) {oldCurs_gu3[i] = newCurs_gu3[i];}
  fr(i,2) {oldScroll_gu2[i] = newScroll_gu2[i];}
}
