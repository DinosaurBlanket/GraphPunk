
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

typedef void (*cursEventHandler)(void *data);
typedef struct {
  cursEventHandler onClickDn;
  cursEventHandler onDrag;
  cursEventHandler onClickUp;
} uiElement;

typedef enum {pei_oport, pei_iport, pei_aface, pei_mface, pei_mhandle} planeElemId;

typedef enum {dt_n, dt_b, dt_na, dt_ba} dtype;

typedef struct {
  planeElemId    pei;
  uiElement      uie;
  dtype          type;
  struct vinode *node;     // part of plane's vinode array
  uint8_t        position; // 0 is leftmost space, 1 is space right adjacent to 0, etc.
  uint8_t        index;    // 0 is leftmost port, 1 is 2nd leftmost port, etc.
  uint8_t        patchCount; // only outputs have more than 1
} port;

typedef struct {
  planeElemId    pei;
  uiElement      uie;
  struct vinode *dnStream; // part of plane's vinode array
  struct vinode *upStream; // part of plane's vinode array
  float         *lineVerts; // part of plane's vert data, both ends of oput pats only
  float         *nodeVerts; // part of plane's vert data, ports and faces
  port          *ports;     // part of plane's vert data, iputs then oputs
  uint8_t        oputCount;
  uint8_t        iputCount;
  uint8_t        totalPatchCount; // sum of all patsPerOput
} vinode;

typedef struct {
  float      rect_gu[4];
  float      pos_gudc2[2]; // only used when changing planes
  GLuint     vao;
  GLuint     vbo;
  GLuint     ebo;
  float     *vertData;
  uint32_t  *indxData;
  uint32_t   lineVertsSize; // in elements (floats)
  uint32_t   lineVertsCap;  // in elements (floats)
  uint32_t   nodeVertsSize; // in elements (ints), ports along with node faces
  uint32_t   nodeVertsCap;  // in elements (ints), ports along with node faces
  vinode    *vinodes;       // malloced memory for all vinode strucutres
  uint32_t   vinodeCount;
  uint32_t   vinodeCap;
  port      *ports;         // malloced memory for all port strucutres
  uint32_t   portCount;
  uint32_t   portCap;
  void      *elements;      // pointers to vinodes and ports that is parallel with vertData
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

bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}

void printVerts(const float *vertData, int vertCount) {
  fr(i,vertCount) {
    printf(
      "%2i: vert pos, gu: % 6.2f, % 6.2f  -  tex pos, nt: % 6.5f, % 6.5f\n",
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

void setRectElems(uint32_t *elems, const uint32_t elemsSize) {
  uint32_t v = 0;
  uint32_t e = 0;
  for (; e < elemsSize; v += 4, e += 6) {
    elems[e  ] = v;
    elems[e+1] = v+1;
    elems[e+2] = v+3;
    elems[e+3] = v+1;
    elems[e+4] = v+2;
    elems[e+5] = v+3;
  }
}

void initPlane() {
  glGenVertexArrays(1, &pln->vao);_glec
  glBindVertexArray(pln->vao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  
  pln->lineVertsSize =   0;
  pln->nodeVertsSize =   0;
  pln->lineVertsCap  = 120; // arbitrary
  pln->nodeVertsCap  =  60; // arbitrary
  pln->pos_gudc2[0] =   0;
  pln->pos_gudc2[1] =   0;
  
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
  resetPlaneRect(pln, halfVideoSize_gu2);
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
  
  setUiVertAttribs(uiShader);
}



void doNothing(void *data) {}
cursEventHandler onDrag    = doNothing;
cursEventHandler onClickUp = doNothing;



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
#define   gc_vertsSize (16*gcid_count) // in elements
#define   gc_indxsSize ( 6*gcid_count) // in elements
GLuint    gc_vao = 0;
GLuint    gc_vbo = 0;
GLuint    gc_ebo = 0;
float    *gc_vertData = NULL;
uint32_t *gc_indxData = NULL;
uiElement gc_uiElems[gcid_count];

bool gc_paused = true;
bool gc_muted  = false;
bool gc_soloed = false;
bool gc_mvBr   = false;
bool gc_locked = false;

bool redrawPlane = true;
bool redrawGc    = true;


void shiftTex(float *vertData, float texBlY, float hight) {
  vertData[ 3] = texBlY +   hight;
  vertData[ 7] = texBlY + 2*hight;
  vertData[11] = texBlY + 2*hight;
  vertData[15] = texBlY +   hight;
}
void unshiftTex(float *vertData, float texBlY, float hight) {
  vertData[ 3] = texBlY;
  vertData[ 7] = texBlY + hight;
  vertData[11] = texBlY + hight;
  vertData[15] = texBlY;
}

void gc_onPlayPauseDn(void *data) {
  if (gc_paused) {
    gc_paused = false;
    shiftTex(&gc_vertData[16*gcid_play], uitex_gc_play_bl_y, uitex_gc_buttonSide);
  }
  else {
    gc_paused = true;
    unshiftTex(&gc_vertData[16*gcid_play], uitex_gc_play_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void gc_onStepDn(void *data) {
  shiftTex(&gc_vertData[16*gcid_step], uitex_gc_step_bl_y, uitex_gc_buttonSide);
  if (!gc_paused) {
    gc_paused = true;
    unshiftTex(&gc_vertData[16*gcid_play], uitex_gc_play_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void gc_onStepUp(void *data) {
  unshiftTex(&gc_vertData[16*gcid_step], uitex_gc_step_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onMuteDn(void *data) {
  if (gc_muted) {
    gc_muted = false;
    unshiftTex(&gc_vertData[16*gcid_mute], uitex_gc_unmuted_bl_y, uitex_gc_buttonSide);
  }
  else {
    gc_muted = true;
    shiftTex(&gc_vertData[16*gcid_mute], uitex_gc_unmuted_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void gc_onSoloDn(void *data) {
  if (gc_soloed) {
    gc_soloed = false;
    unshiftTex(&gc_vertData[16*gcid_solo], uitex_gc_unsoloed_bl_y, uitex_gc_buttonSide);
  }
  else {
    gc_soloed = true;
    shiftTex(&gc_vertData[16*gcid_solo], uitex_gc_unsoloed_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void gc_onMoveBranchDn(void *data) {
  if (gc_mvBr) {
    gc_mvBr = false;
    unshiftTex(&gc_vertData[16*gcid_mvBr], uitex_gc_moveNode_bl_y, uitex_gc_buttonSide);
  }
  else {
    gc_mvBr = true;
    shiftTex(&gc_vertData[16*gcid_mvBr], uitex_gc_moveNode_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void gc_onLockDn(void *data) {
  if (gc_locked) {
    gc_locked = false;
    unshiftTex(&gc_vertData[16*gcid_lock], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  }
  else {
    gc_locked = true;
    shiftTex(&gc_vertData[16*gcid_lock], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void gc_onUpDn(void *data) {
  shiftTex(&gc_vertData[16*gcid_up], uitex_gc_up_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onUpUp(void *data) {
  unshiftTex(&gc_vertData[16*gcid_up], uitex_gc_up_bl_y, uitex_gc_buttonSide
  );
  redrawGc = true;
}
void gc_onTopDn(void *data) {
  shiftTex(&gc_vertData[16*gcid_top], uitex_gc_top_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onTopUp(void *data) {
  unshiftTex(&gc_vertData[16*gcid_top], uitex_gc_top_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onBackDn(void *data) {
  shiftTex(&gc_vertData[16*gcid_back], uitex_gc_back_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onBackUp(void *data) {
  unshiftTex(&gc_vertData[16*gcid_back], uitex_gc_back_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onForwardDn(void *data) {
  shiftTex(&gc_vertData[16*gcid_frwd], uitex_gc_forward_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onForwardUp(void *data) {
  unshiftTex(&gc_vertData[16*gcid_frwd], uitex_gc_forward_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onSaveDn(void *data) {
  shiftTex(&gc_vertData[16*gcid_save], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void gc_onSaveUp(void *data) {
  unshiftTex(&gc_vertData[16*gcid_save], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}

cursEventHandler gc_onClickDns[gcid_count] = {
  gc_onPlayPauseDn,
  gc_onStepDn,
  gc_onMuteDn,
  gc_onSoloDn,
  gc_onMoveBranchDn,
  gc_onLockDn,
  gc_onUpDn,
  gc_onTopDn,
  gc_onBackDn,
  gc_onForwardDn,
  gc_onSaveDn
};
cursEventHandler gc_onDrags[gcid_count] = {
  doNothing,doNothing,doNothing,doNothing,doNothing,doNothing,
  doNothing,doNothing,doNothing,doNothing,doNothing
};
cursEventHandler gc_onClickUps[gcid_count] = {
  doNothing,
  gc_onStepUp,
  doNothing,
  doNothing,
  doNothing,
  doNothing,
  gc_onUpUp,
  gc_onTopUp,
  gc_onBackUp,
  gc_onForwardUp,
  gc_onSaveUp
};


void initGc() {
  glGenVertexArrays(1, &gc_vao);_glec
  glBindVertexArray(gc_vao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  
  glGenBuffers(1, &gc_vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, gc_vbo);_glec
  GLbitfield bufferStorageFlags = 
    GL_MAP_WRITE_BIT      | 
    GL_MAP_PERSISTENT_BIT | 
    GL_MAP_COHERENT_BIT
  ;
  glBufferStorage(
    GL_ARRAY_BUFFER,
    gc_vertsSize*sizeof(float),
    0,
    bufferStorageFlags
  );_glec
  gc_vertData = glMapBufferRange(
    GL_ARRAY_BUFFER,
    0,
    gc_vertsSize*sizeof(float),
    bufferStorageFlags
  );_glec
  
  gc_rect_gu[0] = -gc_butSide_gu*(gcid_count/2.0f);
  gc_rect_gu[1] = halfVideoSize_gu2[1] - gc_butSide_gu;
  gc_rect_gu[2] =  gc_butSide_gu*(gcid_count/2.0f);
  gc_rect_gu[3] = halfVideoSize_gu2[1];
  
  float butRect[4];
  butRect[1] = gc_rect_gu[1];
  butRect[3] = gc_rect_gu[3];
  float texRect[4];
  fr(b, gcid_count) {
    butRect[0] = gc_rect_gu[0] + gc_butSide_gu*b;
    butRect[2] = gc_rect_gu[0] + gc_butSide_gu*(b+1);
    texRect[0] = uitexButCorners_nt[2*b    ];
    texRect[1] = uitexButCorners_nt[2*b + 1];
    texRect[2] = uitexButCorners_nt[2*b    ] + uitex_gc_buttonSide;
    texRect[3] = uitexButCorners_nt[2*b + 1] + uitex_gc_buttonSide;
    mapTexRectToVerts(&gc_vertData[16*b], butRect, texRect);
    gc_uiElems[b].onClickDn = gc_onClickDns[b];
    gc_uiElems[b].onDrag    = gc_onDrags[b];
    gc_uiElems[b].onClickUp = gc_onClickUps[b];
  }
  
  glGenBuffers(1, &gc_ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gc_ebo);_glec
  glBufferStorage(
    GL_ELEMENT_ARRAY_BUFFER,
    gc_indxsSize*sizeof(uint32_t),
    0,
    bufferStorageFlags
  );_glec
  gc_indxData = glMapBufferRange(
    GL_ELEMENT_ARRAY_BUFFER,
    0,
    gc_indxsSize*sizeof(uint32_t),
    bufferStorageFlags
  );_glec
  setRectElems(gc_indxData, 6*gcid_count);
  
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
      if (pointIsInVertRect(newCurs_gu3, &gc_vertData[16*b])) {
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
      glDrawElements(GL_TRIANGLES, backElemsSize, GL_UNSIGNED_INT, 0);_glec
      redrawPlane = false;
    }
    glBindVertexArray(gc_vao);_glec
    glUniform2f(unif_scroll, 0, 0);_glec
    glDrawElements(GL_TRIANGLES, 6*gcid_count, GL_UNSIGNED_INT, 0);_glec
    redrawGc = false;
  }
  fr(i,3) {oldCurs_gu3[i] = newCurs_gu3[i];}
  fr(i,2) {oldScroll_gu2[i] = newScroll_gu2[i];}
  glFinish();
}
