

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
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vg->ibo);_glec
  glDrawElements(GL_TRIANGLES, vg->iCount, GL_UNSIGNED_INT, 0);_glec
}

void putUnitSquareVerts(
  uiVert     *dest,
  const float bl_xy_gu[2], // grid units
  const float bl_st_nt[2], // normalized texture coordinates
  float       texOffset_nt
) {
  // tl
  dest[0].x = bl_xy_gu[0];
  dest[0].y = bl_xy_gu[1] + 1.0;
  dest[0].s = bl_st_nt[0];
  dest[0].t = bl_st_nt[1] + texOffset_nt;
  // tr
  dest[1].x = bl_xy_gu[0] + 1.0;
  dest[1].y = bl_xy_gu[1] + 1.0;
  dest[1].s = bl_st_nt[0] + texOffset_nt;
  dest[1].t = bl_st_nt[1] + texOffset_nt;
  // br
  dest[2].x = bl_xy_gu[0] + 1.0;
  dest[2].y = bl_xy_gu[1];
  dest[2].s = bl_st_nt[0] + texOffset_nt;
  dest[2].t = bl_st_nt[1];
  // bl
  dest[3].x = bl_xy_gu[0];
  dest[3].y = bl_xy_gu[1];
  dest[3].s = bl_st_nt[0];
  dest[3].t = bl_st_nt[1];
}

const float planePadding_gu = 12;

void resetPlaneCorners(plane *pln, float halfVideoSize_gu2[2]) {
  pln->corners_gu4[0] = floor(-halfVideoSize_gu2[0] - planePadding_gu); // tl
  pln->corners_gu4[1] = ceil ( halfVideoSize_gu2[1] + planePadding_gu); // tr
  pln->corners_gu4[2] = ceil ( halfVideoSize_gu2[0] + planePadding_gu); // bl
  pln->corners_gu4[3] = floor(-halfVideoSize_gu2[1] - planePadding_gu); // br
  uiVert backVerts[8] = {
    // inside border
    {    // 0 tl
      pln->corners_gu4[0]+1, pln->corners_gu4[1]-1, 
      uitex_ibord_tl_x, uitex_ibord_tl_y
    }, { // 1 tr
      pln->corners_gu4[2]-1, pln->corners_gu4[1]-1, 
      uitex_ibord_tr_x, uitex_ibord_tr_y
    }, { // 2 br
      pln->corners_gu4[2]-1, pln->corners_gu4[3]+1, 
      uitex_ibord_br_x, uitex_ibord_br_y
    }, { // 3 bl
      pln->corners_gu4[0]+1, pln->corners_gu4[3]+1, 
      uitex_ibord_bl_x, uitex_ibord_bl_y
    },
    // outside border
    {    // 4 tl
      pln->corners_gu4[0], pln->corners_gu4[1], 
      uitex_obord_tl_x, uitex_obord_tl_y
    }, { // 5 tr
      pln->corners_gu4[2], pln->corners_gu4[1], 
      uitex_obord_tr_x, uitex_obord_tr_y
    }, { // 6 br
      pln->corners_gu4[2], pln->corners_gu4[3], 
      uitex_obord_br_x, uitex_obord_br_y
    }, { // 7 bl
      pln->corners_gu4[0], pln->corners_gu4[3], 
      uitex_obord_bl_x, uitex_obord_bl_y
    }
  };
  glBindBuffer(GL_ARRAY_BUFFER, pln->vg.vbo);_glec
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backVerts), backVerts);_glec
}

void initPlane(plane *pln, float halfVideoSize_gu2[2]) {
  pln->vg.vCap = 128;
  pln->vg.iCap = pln->vg.vCap/2;
  glGenBuffers(1, &pln->vg.vbo);_glec
  glGenBuffers(1, &pln->vg.ibo);_glec
  glBindBuffer(GL_ARRAY_BUFFER,         pln->vg.vbo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pln->vg.ibo);_glec
  glBufferData(
    GL_ARRAY_BUFFER,               // GLenum        target
    pln->vg.vCap*sizeof(uiVert),   // GLsizeiptr    size
    0,                             // const GLvoid *data
    GL_STATIC_DRAW                 // GLenum        usage​
  );_glec
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    pln->vg.iCap*sizeof(uint32_t),
    0,
    GL_STATIC_DRAW
  );_glec
  
  pln->vg.vCount = 12;
  resetPlaneCorners(pln, halfVideoSize_gu2);
  uiVert centerVerts[4];
  const float centerVerts_bl_xy_gu[2] = {-0.5, -0.5};
  const float centerVerts_bl_st_nt[2] = {uitex_cntr_bl_x, uitex_cntr_bl_y};
  putUnitSquareVerts(
    centerVerts,
    centerVerts_bl_xy_gu,
    centerVerts_bl_st_nt,
    uitex_guSize_nt
  );
  glBufferSubData(
    GL_ARRAY_BUFFER,
    8*sizeof(uiVert),
    sizeof(centerVerts),
    centerVerts
  );_glec
  
  uint32_t backInd[] = {
    // inside border
    0,1,3, 1,2,3,
    // outside border
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
    // center marker
    8,9,11, 9,10,11
  };
  pln->vg.iCount = sizeof(backInd)/sizeof(uint32_t);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(backInd), backInd);_glec
}
