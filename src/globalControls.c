
#include <stdint.h>
#include <stdbool.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#include "error.h"
#include "../img/uitex.h"
#include "forUi.h"

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

bool onClickGc(float curs_gu[3]) {
  if (pointIsInRect(curs_gu, gc_rect_gu)) {
    fr(b,gcid_count) {
      if (pointIsInVertRect(curs_gu, &gc_vertData[16*b])) {
        onDrag    = gc_uiElems[b].onDrag;
        onClickUp = gc_uiElems[b].onClickUp;
        gc_uiElems[b].onClickDn(NULL);
        return true;
      }
    }
  }
  return false;
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

void initGc(void) {
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
  
  setUiVertAttribs();
}

void drawGc(void) {
  glBindVertexArray(gc_vao);_glec
  glUniform2f(unif_scroll, 0, 0);_glec
  glDrawElements(GL_TRIANGLES, 6*gcid_count, GL_UNSIGNED_INT, 0);_glec
  redrawGc = false;
}
