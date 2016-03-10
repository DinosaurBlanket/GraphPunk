

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#include "error.h"
#include "../img/uitex.h"
#include "misc.h"


bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}


void drawVertGroup(vertGroup *vg) {
  glBindBuffer(GL_ARRAY_BUFFER,         vg->vbo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vg->ebo);_glec
  glDrawElements(GL_TRIANGLES, vg->eCount, GL_UNSIGNED_INT, 0);_glec
}

void mapTexRectToVerts(
  uiVert     *destVerts,
  const float destCorners_gu[4], // grid units
  const float srcCorners_nt[4]   // normalized texture coordinates
) {
  // bl
  destVerts[0].x = destCorners_gu[0];
  destVerts[0].y = destCorners_gu[1];
  destVerts[0].s = srcCorners_nt[0];
  destVerts[0].t = srcCorners_nt[1];
  // tl
  destVerts[1].x = destCorners_gu[0];
  destVerts[1].y = destCorners_gu[3];
  destVerts[1].s = srcCorners_nt[0];
  destVerts[1].t = srcCorners_nt[3];
  // tr
  destVerts[2].x = destCorners_gu[2];
  destVerts[2].y = destCorners_gu[3];
  destVerts[2].s = srcCorners_nt[2];
  destVerts[2].t = srcCorners_nt[3];
  // br
  destVerts[3].x = destCorners_gu[2];
  destVerts[3].y = destCorners_gu[1];
  destVerts[3].s = srcCorners_nt[2];
  destVerts[3].t = srcCorners_nt[1];
}

const float planePadding_gu = 12;

void resetPlaneCorners(plane *pln, float halfVideoSize_gu2[2]) {
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
  glBindBuffer(GL_ARRAY_BUFFER, pln->vg.vbo);_glec
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backVerts), backVerts);_glec
}

void initVertGroup(vertGroup *vg, uint32_t vCap, uint32_t eCap) {
  vg->vCap = vCap;
  vg->eCap = eCap;
  glGenBuffers(1, &vg->vbo);_glec
  glGenBuffers(1, &vg->ebo);_glec
  glBindBuffer(GL_ARRAY_BUFFER,         vg->vbo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vg->ebo);_glec
  glBufferData(
    GL_ARRAY_BUFFER,           // GLenum        target
    vg->vCap*sizeof(uiVert),   // GLsizeiptr    size
    0,                         // const GLvoid *data
    GL_STATIC_DRAW             // GLenum        usage​
  );_glec
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    vg->eCap*sizeof(uint32_t),
    0,
    GL_STATIC_DRAW
  );_glec
}

void initPlane(plane *pln, float halfVideoSize_gu2[2]) {
  initVertGroup(&pln->vg, 128, 192);
  
  pln->vg.vCount = 12;
  resetPlaneCorners(pln, halfVideoSize_gu2);
  uiVert centerVerts[4];
  const float centerVertsCorners_gu[4] = {-1, -1, 1, 1};
  const float centerTexCorners_nt[4] = {
    uitex_cntr_bl_x, uitex_cntr_bl_y, uitex_cntr_tr_x, uitex_cntr_tr_y
  };
  mapTexRectToVerts(centerVerts, centerVertsCorners_gu, centerTexCorners_nt);
  
  glBufferSubData(
    GL_ARRAY_BUFFER,
    8*sizeof(uiVert),
    sizeof(centerVerts),
    centerVerts
  );_glec
  
  uint32_t backElem[] = {
    // inside border
    0,1,3, 1,2,3,
    // outside border
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
    // center marker
    8,9,11, 9,10,11
  };
  pln->vg.eCount = sizeof(backElem)/sizeof(uint32_t);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(backElem), backElem);_glec
}

void setRectElems(uint32_t *elems, uint32_t rectCount) {
  for (uint32_t i = 0; i < 6*rectCount; i += 6) {
    elems[i  ] = i;
    elems[i+1] = i+1;
    elems[i+2] = i+3;
    elems[i+3] = i+1;
    elems[i+4] = i+2;
    elems[i+5] = i+3;
  }
}

void initGlorolsVerts(vertGroup *vg, float halfVideoSize_gu2[2]) {
  const int visibleButCount = 11;
  const float butSide_gu = 2.0f;
  const float butLeftEdge_gu = 0.0f - butSide_gu*(visibleButCount/2.0f);
  const int vertCount = 4*visibleButCount;
  const int elemCount = 6*visibleButCount;
  int   vertIndex = 0;
  uiVert verts[vertCount];
  float vertCorners[4] = {
    butLeftEdge_gu,              halfVideoSize_gu2[1] - butSide_gu, 
    butLeftEdge_gu + butSide_gu, halfVideoSize_gu2[1]
  };
  float texCorners[4];
  
  texCorners[0] = uitex_gc_play_bl_x;
  texCorners[1] = uitex_gc_play_bl_y;
  texCorners[2] = uitex_gc_play_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_play_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_step_bl_x;
  texCorners[1] = uitex_gc_step_bl_y;
  texCorners[2] = uitex_gc_step_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_step_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_unmuted_bl_x;
  texCorners[1] = uitex_gc_unmuted_bl_y;
  texCorners[2] = uitex_gc_unmuted_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_unmuted_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_unsoloed_bl_x;
  texCorners[1] = uitex_gc_unsoloed_bl_y;
  texCorners[2] = uitex_gc_unsoloed_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_unsoloed_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_moveNode_bl_x;
  texCorners[1] = uitex_gc_moveNode_bl_y;
  texCorners[2] = uitex_gc_moveNode_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_moveNode_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_unLock_bl_x;
  texCorners[1] = uitex_gc_unLock_bl_y;
  texCorners[2] = uitex_gc_unLock_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_unLock_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_up_bl_x;
  texCorners[1] = uitex_gc_up_bl_y;
  texCorners[2] = uitex_gc_up_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_up_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_top_bl_x;
  texCorners[1] = uitex_gc_top_bl_y;
  texCorners[2] = uitex_gc_top_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_top_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_back_bl_x;
  texCorners[1] = uitex_gc_back_bl_y;
  texCorners[2] = uitex_gc_back_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_back_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_forward_bl_x;
  texCorners[1] = uitex_gc_forward_bl_y;
  texCorners[2] = uitex_gc_forward_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_forward_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  vertIndex += 4;
  vertCorners[0] += butSide_gu;
  vertCorners[2] += butSide_gu;
  texCorners[0] = uitex_gc_save_bl_x;
  texCorners[1] = uitex_gc_save_bl_y;
  texCorners[2] = uitex_gc_save_bl_x + uitex_gc_buttonSide;
  texCorners[3] = uitex_gc_save_bl_y + uitex_gc_buttonSide;
  mapTexRectToVerts(&verts[vertIndex], vertCorners, texCorners);
  
  puts("verts:");
  printVerts(verts, vertCount);
  
  initVertGroup(vg, vertCount, elemCount);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);_glec
  
  uint32_t elems[elemCount];
  setRectElems(elems, visibleButCount);
  puts("\nelems");
  fr(i,elemCount) {printf("%2i: %2i\n", i, elems[i]);}
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(elems), elems);_glec
}
