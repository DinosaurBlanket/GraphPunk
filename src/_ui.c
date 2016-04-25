
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#include "error.h"
#include "oglTools.h"
#include "uitex.h"
#include "misc.h"


#include "plane.h"
// from old plane struct
float      rect[4];         // borders of plane
float      pos[2];          // only used when changing planes
planeElem *planeElems;
uint32_t   planeElemCount;
uint32_t   planeElemCap;
float     *vertData;        // GL buffer storage, 16 floats(4 verts) per rect
uint32_t  *indxData;        // GL buffer storage,  6 ints per rect
GLuint     vao;
GLuint     vbo;
GLuint     ebo;



// new
#define gcButtonCount 10






//#include "forUi.h"
#define fingerUnit 16
float halfVideoSize_2[2] = {0};
void printVerts(const float *vertData, int vertCount) {
  fr(i,vertCount) {
    printf(
      "%2i: vert pos, gu: % 6.2f, % 6.2f  -  tex pos, nt: % 6.5f, % 6.5f\n",
      i, vertData[4*i], vertData[4*i + 1], vertData[4*i + 2], vertData[4*i + 3]
    );
  }
}
typedef void (*cursEventHandler)(void *data);
extern void doNothing(void *data) {}
cursEventHandler onDrag    = doNothing;
cursEventHandler onClickUp = doNothing;
void mapTexRectToVerts(
  float      *destVertData,
  const float destRect_px[4], // grid units
  const float srcRect_nt[4]   // normalized texture coordinates
) {
  // bl
  destVertData[ 0] = destRect_px[0];
  destVertData[ 1] = destRect_px[1];
  destVertData[ 2] =  srcRect_nt[0];
  destVertData[ 3] =  srcRect_nt[1];
  // tl
  destVertData[ 4] = destRect_px[0];
  destVertData[ 5] = destRect_px[3];
  destVertData[ 6] =  srcRect_nt[0];
  destVertData[ 7] =  srcRect_nt[3];
  // tr
  destVertData[ 8] = destRect_px[2];
  destVertData[ 9] = destRect_px[3];
  destVertData[10] =  srcRect_nt[2];
  destVertData[11] =  srcRect_nt[3];
  // br
  destVertData[12] = destRect_px[2];
  destVertData[13] = destRect_px[1];
  destVertData[14] =  srcRect_nt[2];
  destVertData[15] =  srcRect_nt[1];
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
GLuint uiShader;
GLuint uiTex;
GLint unif_scroll;
GLint unif_halfVideoSize;
void setUiVertAttribs(void) {
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

const GLbitfield bufferStorageFlags =
  GL_MAP_WRITE_BIT      |
  GL_MAP_PERSISTENT_BIT |
  GL_MAP_COHERENT_BIT
;






//#include "saveLoad.h"
module *loadModules(uint32_t id) {
  module *m;
  if (id == rootModId) m = &root;
  else {
    // m = malloc module
    // load module data from disk
    // initialize id, parent, toggles, faceRect
  }
  plane *p = &m->plane;

  // initialize some critical plane data
  p->planeElemCount = lastModHeader.planeElemCount;
  if (!lastModHeader.planeElemCount) {
    p->planeElemCap = 0;
    p->planeElems   = NULL;
    return m;
  }
  p->planeElemCap = nextHighestPO2(lastModHeader.planeElemCount);

  uint32_t lineCount = 0;
  fr(i, p->planeElemCount) {
    if (p->planeElems[i].pei == pei_iport && p->planeElems[i].p.ocon) {
      lineCount++;
    }
  }
  p->nodeVertsOffset = initialNodeVertsOffset;
  while (
    16*lineCount         > p->nodeVertsOffset - backVertsSize ||
    16*p->planeElemCount > p->nodeVertsOffset
  ) {
    p->nodeVertsOffset = nextHighestPO2(p->nodeVertsOffset);
  }
  // first 1/2 of data is for background and lines, 2nd 1/2 is for planeElems
  const uint32_t bufSize = 2 * p->nodeVertsOffset * sizeof(float);

  glGenVertexArrays(1, &p->vao);_glec
  glBindVertexArray(p->vao);_glec
  glGenBuffers(1, &p->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, p->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, p->vbo);_glec
  glBufferStorage(GL_ARRAY_BUFFER, bufSize, 0, bufferStorageFlags);_glec

  p->vertData = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufSize, bufferStorageFlags);_glec
  p->planeElems = malloc(p->planeElemCap*sizeof(planeElem));

  fr(i, p->planeElemCount) {
    p->planeElems[i] = lastModPlaneElems[i];
    if (p->planeElems[i].pei == pei_mface) {
      p->planeElems[i].n.module = (struct module*)loadModules(p->planeElems[i].n.id); // why the hell do i need to cast this??
    }
    p->vertData[16*i    ] = lastModPlaneElemPositions[2*i];
    p->vertData[16*i + 1] = lastModPlaneElemPositions[2*i + 1];
  }

  return m;
}






//#include "globalControls.h"
#define butSide uitex_gc_buttonSide
float wholeRect[4] = {
  uitex_gc_bl_x, uitex_gc_bl_y,
  uitex_gc_tr_x, uitex_gc_tr_y
};
typedef enum {
  gcid_play,
  gcid_step,
  gcid_compile,
  gcid_vidZoom,
  gcid_nodeZoom,
  gcid_actBranch,
  gcid_lock,
  gcid_revert,
  gcid_save,
  gcid_exit, gcidCount
} gcid;
const float uitexButCorners_nt[2*gcidCount] = {
  uitex_gc_play_bl_x,      uitex_gc_play_bl_y,
  uitex_gc_step_bl_x,      uitex_gc_step_bl_y,
  uitex_gc_compile_bl_x,   uitex_gc_compile_bl_y,
  uitex_gc_vidZoom_bl_x,   uitex_gc_vidZoom_bl_y,
  uitex_gc_nodeZoom_bl_x,  uitex_gc_nodeZoom_bl_y,
  uitex_gc_actBranch_bl_x, uitex_gc_actBranch_bl_y,
  uitex_gc_lock_bl_x,      uitex_gc_lock_bl_y,
  uitex_gc_revert_bl_x,    uitex_gc_revert_bl_y,
  uitex_gc_save_bl_x,      uitex_gc_save_bl_y,
  uitex_gc_exit_bl_x,      uitex_gc_exit_bl_y
};
#define   vertsSize (16*gcid_count) // in elements
#define   indxsSize ( 6*gcid_count) // in elements
GLuint    vao = 0;
GLuint    vbo = 0;
GLuint    ebo = 0;
bool paused = true;
bool muted  = false;
bool soloed = false;
bool mvBr   = false;
bool locked = false;
bool redrawGc = true;
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
void onPlayPauseDn(void *data) {
  if (paused) {
    paused = false;
    shiftTex(&vertData[16*gcid_play], uitex_gc_play_bl_y, uitex_gc_buttonSide);
  }
  else {
    paused = true;
    unshiftTex(&vertData[16*gcid_play], uitex_gc_play_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void onStepDn(void *data) {
  shiftTex(&vertData[16*gcid_step], uitex_gc_step_bl_y, uitex_gc_buttonSide);
  if (!paused) {
    paused = true;
    unshiftTex(&vertData[16*gcid_play], uitex_gc_play_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void onStepUp(void *data) {
  unshiftTex(&vertData[16*gcid_step], uitex_gc_step_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onMuteDn(void *data) {
  if (muted) {
    muted = false;
    unshiftTex(&vertData[16*gcid_mute], uitex_gc_unmuted_bl_y, uitex_gc_buttonSide);
  }
  else {
    muted = true;
    shiftTex(&vertData[16*gcid_mute], uitex_gc_unmuted_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void onSoloDn(void *data) {
  if (soloed) {
    soloed = false;
    unshiftTex(&vertData[16*gcid_solo], uitex_gc_unsoloed_bl_y, uitex_gc_buttonSide);
  }
  else {
    soloed = true;
    shiftTex(&vertData[16*gcid_solo], uitex_gc_unsoloed_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void onMoveBranchDn(void *data) {
  if (mvBr) {
    mvBr = false;
    unshiftTex(&vertData[16*gcid_mvBr], uitex_gc_moveNode_bl_y, uitex_gc_buttonSide);
  }
  else {
    mvBr = true;
    shiftTex(&vertData[16*gcid_mvBr], uitex_gc_moveNode_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void onLockDn(void *data) {
  if (locked) {
    locked = false;
    unshiftTex(&vertData[16*gcid_lock], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  }
  else {
    locked = true;
    shiftTex(&vertData[16*gcid_lock], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  }
  redrawGc = true;
}
void onUpDn(void *data) {
  shiftTex(&vertData[16*gcid_up], uitex_gc_up_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onUpUp(void *data) {
  unshiftTex(&vertData[16*gcid_up], uitex_gc_up_bl_y, uitex_gc_buttonSide
  );
  redrawGc = true;
}
void onTopDn(void *data) {
  shiftTex(&vertData[16*gcid_top], uitex_gc_top_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onTopUp(void *data) {
  unshiftTex(&vertData[16*gcid_top], uitex_gc_top_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onBackDn(void *data) {
  shiftTex(&vertData[16*gcid_back], uitex_gc_back_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onBackUp(void *data) {
  unshiftTex(&vertData[16*gcid_back], uitex_gc_back_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onForwardDn(void *data) {
  shiftTex(&vertData[16*gcid_frwd], uitex_gc_forward_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onForwardUp(void *data) {
  unshiftTex(&vertData[16*gcid_frwd], uitex_gc_forward_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onSaveDn(void *data) {
  shiftTex(&vertData[16*gcid_save], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
}
void onSaveUp(void *data) {
  unshiftTex(&vertData[16*gcid_save], uitex_gc_unLock_bl_y, uitex_gc_buttonSide);
  redrawGc = true;
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
bool onClickGc(float curs[3]) {
  if (pointIsInRect(curs, wholeRect)) {
    fr(b,gcid_count) {
      if (pointIsInVertRect(curs, &vertData[16*b])) {
        onDrag    = uiElems[b].onDrag;
        onClickUp = uiElems[b].onClickUp;
        uiElems[b].onClickDn(NULL);
        return true;
      }
    }
  }
  return false;
}
cursEventHandler onClickDns[gcid_count] = {
  onPlayPauseDn,
  onStepDn,
  onMuteDn,
  onSoloDn,
  onMoveBranchDn,
  onLockDn,
  onUpDn,
  onTopDn,
  onBackDn,
  onForwardDn,
  onSaveDn
};
cursEventHandler onDrags[gcid_count] = {
  doNothing,doNothing,doNothing,doNothing,doNothing,doNothing,
  doNothing,doNothing,doNothing,doNothing,doNothing
};
cursEventHandler onClickUps[gcid_count] = {
  doNothing,
  onStepUp,
  doNothing,
  doNothing,
  doNothing,
  doNothing,
  onUpUp,
  onTopUp,
  onBackUp,
  onForwardUp,
  onSaveUp
};
void initGc(void) {
  glGenVertexArrays(1, &vao);_glec
  glBindVertexArray(vao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec

  glGenBuffers(1, &vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, vbo);_glec
  GLbitfield bufferStorageFlags =
    GL_MAP_WRITE_BIT      |
    GL_MAP_PERSISTENT_BIT |
    GL_MAP_COHERENT_BIT
  ;
  glBufferStorage(
    GL_ARRAY_BUFFER,
    vertsSize*sizeof(float),
    0,
    bufferStorageFlags
  );_glec
  vertData = glMapBufferRange(
    GL_ARRAY_BUFFER,
    0,
    vertsSize*sizeof(float),
    bufferStorageFlags
  );_glec

  wholeRect[0] = -butSide*(gcid_count/2.0f);
  wholeRect[1] = halfVideoSize_2[1] - butSide;
  wholeRect[2] =  butSide*(gcid_count/2.0f);
  wholeRect[3] = halfVideoSize_2[1];

  float butRect[4];
  butRect[1] = wholeRect[1];
  butRect[3] = wholeRect[3];
  float texRect[4];
  fr(b, gcid_count) {
    butRect[0] = wholeRect[0] + butSide*b;
    butRect[2] = wholeRect[0] + butSide*(b+1);
    texRect[0] = uitexButCorners_nt[2*b    ];
    texRect[1] = uitexButCorners_nt[2*b + 1];
    texRect[2] = uitexButCorners_nt[2*b    ] + uitex_gc_buttonSide;
    texRect[3] = uitexButCorners_nt[2*b + 1] + uitex_gc_buttonSide;
    mapTexRectToVerts(&vertData[16*b], butRect, texRect);
    uiElems[b].onClickDn = onClickDns[b];
    uiElems[b].onDrag    = onDrags[b];
    uiElems[b].onClickUp = onClickUps[b];
  }

  glGenBuffers(1, &ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);_glec
  glBufferStorage(
    GL_ELEMENT_ARRAY_BUFFER,
    indxsSize*sizeof(uint32_t),
    0,
    bufferStorageFlags
  );_glec
  indxData = glMapBufferRange(
    GL_ELEMENT_ARRAY_BUFFER,
    0,
    indxsSize*sizeof(uint32_t),
    bufferStorageFlags
  );_glec
  setRectElems(indxData, 6*gcid_count);

  setUiVertAttribs();
}
void drawGc(void) {
  glBindVertexArray(vao);_glec
  glUniform2f(unif_scroll, 0, 0);_glec
  glDrawElements(GL_TRIANGLES, 6*gcid_count, GL_UNSIGNED_INT, 0);_glec
  redrawGc = false;
}









module *mdl = NULL;
plane  *pln = NULL;
const float planePadding = 12*fingerUnit; // arbitrary


void resetPlaneRect(void) {
  float halfPlaneSize[2];
  fr(i,2) {
    halfPlaneSize[i] =
      ceil(halfVideoSize_2[i]/fingerUnit)*fingerUnit + planePadding
    ;
  }
  pln->rect[0] = -halfPlaneSize[0];
  pln->rect[1] = -halfPlaneSize[1];
  pln->rect[2] =  halfPlaneSize[0];
  pln->rect[3] =  halfPlaneSize[1];
  const float backVertData[32] = {
    // inside border
    // bl
    pln->rect[0]+fingerUnit, pln->rect[1]+fingerUnit,
    uitex_ibord_bl_x, uitex_ibord_bl_y,
    // tl
    pln->rect[0]+fingerUnit, pln->rect[3]-fingerUnit,
    uitex_ibord_tl_x, uitex_ibord_tl_y,
    // tr
    pln->rect[2]-fingerUnit, pln->rect[3]-fingerUnit,
    uitex_ibord_tr_x, uitex_ibord_tr_y,
    // br
    pln->rect[2]-fingerUnit, pln->rect[1]+fingerUnit,
    uitex_ibord_br_x, uitex_ibord_br_y,
    // outside border
    // bl
    pln->rect[0], pln->rect[1],
    uitex_obord_bl_x, uitex_obord_bl_y,
    // tl
    pln->rect[0], pln->rect[3],
    uitex_obord_tl_x, uitex_obord_tl_y,
    // tr
    pln->rect[2], pln->rect[3],
    uitex_obord_tr_x, uitex_obord_tr_y,
    // br
    pln->rect[2], pln->rect[1],
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
  unif_scroll        = glGetUniformLocation(uiShader, "scroll");_glec
  unif_halfVideoSize = glGetUniformLocation(uiShader, "halfVideoSize");_glec
  glUniform2f(unif_halfVideoSize, halfVideoSize_2[0], halfVideoSize_2[1]);_glec
  glUniform2f(unif_scroll, 0, 0);_glec
  glGenTextures(1, &uiTex);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  texFromBmp(uitex_path);
  glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);_glec
}

void initPlane(void) {




  pln = &mdl->plane;
  glBindVertexArray(pln->vao);_glec
  glUseProgram(uiShader);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec


  // vertData glBufferStorage allocation used to be here


  glGenBuffers(1, &pln->ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pln->ebo);_glec
  uint32_t bufSize = nextHighestPO2((backElemsSize + (6*pln->planeElemCap)) * sizeof(uint32_t));
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, bufSize, 0, bufferStorageFlags);_glec
  pln->indxData = glMapBufferRange(
    GL_ELEMENT_ARRAY_BUFFER, 0, bufSize, bufferStorageFlags
  );_glec

  // the first 8 verts for the background are set by this function
  resetPlaneRect();
  // the next 4 for the center marker are set below
  float centerVerts[16];
  const float centerVertsRect[4] = {
    -fingerUnit, -fingerUnit, fingerUnit, fingerUnit
  };
  const float centerTexRect_nt[4] = {
    uitex_cntr_bl_x, uitex_cntr_bl_y, uitex_cntr_tr_x, uitex_cntr_tr_y
  };
  mapTexRectToVerts(centerVerts, centerVertsRect, centerTexRect_nt);
  for (int i = backVertsSize-16; i < backVertsSize; i++) {
    pln->vertData[i] = centerVerts[i-(backVertsSize-16)];
  }
  // next is the lineVerts

  // then finally the nodeVerts
  fr(i, pln->planeElemCount) {
    switch(pln->planeElems[i].pei) {
      case pei_iport:
      case pei_oport:
      case pei_aface:
      case pei_mface:
      default: _SHOULD_NOT_BE_HERE_;
    }
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

void initUi(float videoSize_px2[2]) {
  fr(i,2) {halfVideoSize_2[i] = videoSize_px2[i]/2.0f;}
  mdl = saveLoadInit();
  initUiShader();
  initPlane();
  initGc();
}

float newCurs_3[3]       = {0}; // cursor state relative to screen
float oldCurs_3[3]       = {0};
float clickDnCurs_3[3]   = {0};
float newScroll_2[2]     = {0}; // plane center to screen center difference
float oldScroll_2[2]     = {0};
float clickDnScroll_2[2] = {0};
float scrollVel_2[2]     = {0};

void onDragScroll(void *data) {
  fr(i,2) {
    newScroll_2[i] = clickDnScroll_2[i]-(clickDnCurs_3[i]-newCurs_3[i]);
  }
}
void onClickUpScroll(void *data) {
  fr(i,2) {scrollVel_2[i] = newCurs_3[i] - oldCurs_3[i];}
}

void corneredToCentered(float centered[2], const int cX, const int cY) {
  centered[0] =  cX - halfVideoSize_2[0];
  centered[1] = -cY + halfVideoSize_2[1];
}

void clickDn(int posX_px, int posY_px) {
  corneredToCentered(newCurs_3, posX_px, posY_px);
  newCurs_3[2] = 1.0f;
  fr(i,3) {clickDnCurs_3[i] = newCurs_3[i];}
  fr(i,2) {clickDnScroll_2[i] = newScroll_2[i];}
  if (!onClickGc(newCurs_3)) {
    fr(i,2) {scrollVel_2[i] = 0;}
    onDrag    = onDragScroll;
    onClickUp = onClickUpScroll;
  }
}
void curMove(int posX_px, int posY_px) {
  corneredToCentered(newCurs_3, posX_px, posY_px);
  if (newCurs_3[2]) onDrag(NULL);
}
void clickUp(int posX_px, int posY_px) {
  corneredToCentered(newCurs_3, posX_px, posY_px);
  newCurs_3[2] = 0;
  onClickUp(NULL);
}

float screenCrnrs_4[4]  = {0}; // xyxy, bl tr, relative to plane center
bool redrawPlane = true;

void perFrame(void) {
  fr(i,2) {newScroll_2[i] += scrollVel_2[i];}
  if (!allEq(newScroll_2, oldScroll_2, 2)) {
    screenCrnrs_4[0] = newScroll_2[0] - halfVideoSize_2[0];
    screenCrnrs_4[1] = newScroll_2[1] - halfVideoSize_2[1];
    screenCrnrs_4[2] = newScroll_2[0] + halfVideoSize_2[0];
    screenCrnrs_4[3] = newScroll_2[1] + halfVideoSize_2[1];
    fr(i,2) {
      if (screenCrnrs_4[i] < pln->rect[i]) {
        newScroll_2[i] = pln->rect[i] + halfVideoSize_2[i];
        scrollVel_2[i] = 0;
      }
      else if (screenCrnrs_4[i+2] > pln->rect[i+2]) {
        newScroll_2[i] = pln->rect[i+2] - halfVideoSize_2[i];
        scrollVel_2[i] = 0;
      }
    }
    redrawPlane = true;
  }
  if (redrawPlane || redrawGc) {
    if (redrawPlane) {
      glBindVertexArray(pln->vao);_glec
      glUniform2f(unif_scroll, newScroll_2[0], newScroll_2[1]);_glec
      glDrawElements(GL_TRIANGLES, backElemsSize, GL_UNSIGNED_INT, 0);_glec
      redrawPlane = false;
    }
    drawGc();
  }
  fr(i,3) {oldCurs_3[i] = newCurs_3[i];}
  fr(i,2) {oldScroll_2[i] = newScroll_2[i];}
  glFinish();
}
