
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#include "error.h"
#include "oglTools.h"
#include "uitex.h"
#include "forUi.h"
#include "globalControls.h"
#include "module.h"
#include "saveLoad.h"
#include "misc.h"


module *mdl = NULL;
plane  *pln = NULL;
const float planePadding = 12*fingerUnit; // arbitrary

#include <stdio.h>
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

void initRoot(float videoSize_px2[2]) {
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
