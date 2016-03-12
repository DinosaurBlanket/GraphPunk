

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
  
  //glBindVertexArray(pln->vao);_glec
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backVerts), backVerts);_glec
}

void initUiVertAttribs(GLuint shader) {
  GLint attr_pos      = glGetAttribLocation(shader, "pos");_glec
  GLint attr_texCoord = glGetAttribLocation(shader, "texCoord");_glec
  glEnableVertexAttribArray(attr_pos  );_glec
  glEnableVertexAttribArray(attr_texCoord);_glec
  glVertexAttribPointer(
    attr_pos,      2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)0
  );_glec
  glVertexAttribPointer(
    attr_texCoord, 2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)8
  );_glec
}

void initPlane(
  plane *pln,
  GLuint shader,
  GLuint tex,
  float  halfVideoSize_gu2[2]
) {
  glGenVertexArrays(1, &pln->vao);_glec
  glBindVertexArray(pln->vao);_glec
  glUseProgram(shader);_glec
  glBindTexture(GL_TEXTURE_2D, tex);_glec
  
  glGenBuffers(1, &pln->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER,         pln->vbo);_glec
  glBufferData(
    GL_ARRAY_BUFFER,               // GLenum        target
    lineVertOffset*sizeof(uiVert), // GLsizeiptr    size
    0,                             // const GLvoid *data
    GL_STATIC_DRAW                 // GLenum        usage​
  );_glec
  // the first 8 verts are set by this function
  resetPlaneCorners(pln, halfVideoSize_gu2);
  // the next 4 are set below
  uiVert centerVerts[4];
  const float centerVertsCorners_gu[4] = {-1.0, -1.0, 1.0, 1.0};
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
  
  initUiVertAttribs(shader);
}

void setRectElems(uint32_t *elems, const uint32_t rectCount) {
  uint32_t v = 0;
  uint32_t e = 0;
  for (; e < 6*rectCount; v += 4, e += 6) {
    elems[e  ] = v;
    elems[e+1] = v+1;
    elems[e+2] = v+3;
    elems[e+3] = v+1;
    elems[e+4] = v+2;
    elems[e+5] = v+3;
  }
}

void initGlorols(
  GLuint vao,
  GLuint shader,
  GLuint tex,
  float  halfVideoSize_gu2[2]
) {
  glBindVertexArray(vao);_glec
  glUseProgram(shader);_glec
  glBindTexture(GL_TEXTURE_2D, tex);_glec
  
  const float butSide_gu = 2.0f;
  const float butLeftEdge_gu = 0.0f - butSide_gu*(glorolsButCount/2.0f);
  const int vertCount = 4*glorolsButCount;
  const int elemCount = 6*glorolsButCount;
  const float uitexButCorners[2*glorolsButCount] = {
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
  float vertCorners[4] = {
    butLeftEdge_gu,              halfVideoSize_gu2[1] - butSide_gu, 
    butLeftEdge_gu + butSide_gu, halfVideoSize_gu2[1]
  };
  uiVert verts[vertCount];
  float texCorners[4];
  fr(b, glorolsButCount) {
    texCorners[0] = uitexButCorners[2*b];
    texCorners[1] = uitexButCorners[2*b + 1];
    texCorners[2] = uitexButCorners[2*b]     + uitex_gc_buttonSide;
    texCorners[3] = uitexButCorners[2*b + 1] + uitex_gc_buttonSide;
    mapTexRectToVerts(&verts[4*b], vertCorners, texCorners);
    vertCorners[0] += butSide_gu;
    vertCorners[2] += butSide_gu;
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
  
  initUiVertAttribs(shader);
}
