
#include <stdint.h>
#include <stdbool.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#include "error.h"
#include "uitex.h"
#include "forUi.h"
#include "misc.h"


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
float    *vertData = NULL;
uint32_t *indxData = NULL;

typedef struct {
  cursEventHandler onClickDn;
  cursEventHandler onDrag;
  cursEventHandler onClickUp;
} uiElement;
uiElement uiElems[gcid_count];

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
