
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#define  GLEW_STATIC
#include <GL/glew.h>
#include "error.h"
#include "forUi.h"

bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}

float halfVideoSize_2[2];

void printVerts(const float *vertData, int vertCount) {
  fr(i,vertCount) {
    printf(
      "%2i: vert pos, gu: % 6.2f, % 6.2f  -  tex pos, nt: % 6.5f, % 6.5f\n",
      i, vertData[4*i], vertData[4*i + 1], vertData[4*i + 2], vertData[4*i + 3]
    );
  }
}

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
