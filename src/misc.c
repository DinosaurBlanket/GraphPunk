

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

void resetPlaneCorners(plane *pln, GLuint uiVao, float halfVideoSize_gu2[2]) {
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
  
  glBindVertexArray(uiVao);_glec
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backVerts), backVerts);_glec
}
