
#include <stdlib.h>
#include <stdio.h>

#include "misc.h"
#include "error.h"
#include "oglTools.h"
#include "planeElem.h"
#include "nodeDef.h"
#include "pretendFile.h"
#include "uitex.h"

float halfVideoSize[2] = {0};
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
void mapTexRectToVertPos(
  float      *destVertData,
  const float destPos[2],
  const float srcRect[4]
) {
  // bl
  destVertData[ 0] = destPos[0];
  destVertData[ 1] = destPos[1];
  destVertData[ 2] = srcRect[0];
  destVertData[ 3] = srcRect[1];
  // tl
  destVertData[ 4] = destPos[0];
  destVertData[ 5] = destPos[1] + (srcRect[3] - srcRect[1]);
  destVertData[ 6] = srcRect[0];
  destVertData[ 7] = srcRect[3];
  // tr
  destVertData[ 8] = destPos[0] + (srcRect[2] - srcRect[0]);
  destVertData[ 9] = destVertData[5];
  destVertData[10] = srcRect[2];
  destVertData[11] = srcRect[3];
  // br
  destVertData[12] = destVertData[8];
  destVertData[13] = destPos[1];
  destVertData[14] = srcRect[2];
  destVertData[15] = srcRect[1];
}
void mapTexRectToVertRect(
  float      *destVertData,
  const float destRect[4],
  const float srcRect[4]
) {
  // bl
  destVertData[ 0] = destRect[0];
  destVertData[ 1] = destRect[1];
  destVertData[ 2] = srcRect[0];
  destVertData[ 3] = srcRect[1];
  // tl
  destVertData[ 4] = destRect[0];
  destVertData[ 5] = destRect[3];
  destVertData[ 6] = srcRect[0];
  destVertData[ 7] = srcRect[3];
  // tr
  destVertData[ 8] = destRect[2];
  destVertData[ 9] = destRect[3];
  destVertData[10] = srcRect[2];
  destVertData[11] = srcRect[3];
  // br
  destVertData[12] = destRect[2];
  destVertData[13] = destRect[1];
  destVertData[14] = srcRect[2];
  destVertData[15] = srcRect[1];
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


float      planeRect[4]   = {0};
float      planePos[2]    = {0};
planeElem *planeElems     = NULL;
uint32_t   planeElemCount = 0;
uint32_t   planeElemCap   = 0;
float     *vertData       = NULL; // GL buffer, 16 floats(4 verts) per rect
uint32_t   vertDataCount  = 0;    // float count
uint32_t   vertDataCap    = 0;    // float count
uint32_t  *indxData       = NULL; // GL buffer,  6 ints per rect
uint32_t   indxDataCount  = 0;    // int count
uint32_t   indxDataCap    = 0;
GLuint vao = 0;
GLuint vbo = 0;
GLuint ebo = 0;
GLint unif_scroll = 0;
GLint unif_halfVideoSize = 0;
const GLbitfield bufferStorageFlags =
  GL_MAP_WRITE_BIT      |
  GL_MAP_PERSISTENT_BIT |
  GL_MAP_COHERENT_BIT
;

#define gcButtonCount        10
#define gcVertDataStart       0
#define gcIndxDataStart       0
#define gcVertDataCount     (gcButtonCount*16)
#define gcIndxDataCount     (gcButtonCount*6)
#define borderVertDataStart (gcVertDataStart + gcVertDataCount)
#define borderIndxDataStart (gcIndxDataStart + gcIndxDataCount)
#define borderVertDataCount  32
#define borderIndxDataCount  24
// there will be more fixed-size vert data later
#define peVertDataStart     (borderVertDataStart + borderVertDataCount)
uint32_t peVertDataCap(void) {return planeElemCap*12 - peVertDataStart;}
uint32_t lineVertDataStart(void) {return peVertDataStart + peVertDataCap();}
uint32_t lineVertDataCap(void) {return planeElemCap*4;}
// The line verts take 1/4 of the entire buffer size,
// and planeElems take the other 3/4 - the fixed-size stuff in the beginning.
// draw order: lines -> border & planeElems -> global controls

void resizeBuffers(void) {
  if (!planeElems) {
    planeElems = malloc(sizeof(planeElem)*planeElemCap);
  }
  else {
    // realloc
  }
  vertDataCap = nextHighestPO2(planeElemCap*16 + peVertDataStart + lineVertDataCap());
  indxDataCap = (vertDataCap/2)*3;
  if (!vertData) {
    if (indxData) _SHOULD_NOT_BE_HERE_;
    glBufferStorage(
      GL_ARRAY_BUFFER,
      vertDataCap*sizeof(float),
      0,
      bufferStorageFlags
    );_glec
    vertData = glMapBufferRange(
      GL_ARRAY_BUFFER,
      0,
      vertDataCap*sizeof(float),
      bufferStorageFlags
    );_glec
    glBufferStorage(
      GL_ELEMENT_ARRAY_BUFFER,
      indxDataCap*sizeof(uint32_t),
      0,
      bufferStorageFlags
    );_glec
    indxData = glMapBufferRange(
      GL_ELEMENT_ARRAY_BUFFER,
      0,
      indxDataCap*sizeof(uint32_t),
      bufferStorageFlags
    );_glec
  }
  else {
    if (!indxData) _SHOULD_NOT_BE_HERE_;
    // copy data to new, bigger buffers
  }
}


#define planePadding  (fingerUnit*12) // arbitrary
void resetPlaneRect(void) {
  // start with the position of the first pe, doesn't matter what pe it is
  planeRect[0] = vertData[peVertDataStart+0];
  planeRect[1] = vertData[peVertDataStart+1];
  planeRect[2] = vertData[peVertDataStart+8];
  planeRect[3] = vertData[peVertDataStart+9];
  // then stretch to include every other pe
  for (int i = peVertDataStart; i < planeElemCount; i += 16) {
    if (vertData[i+0] < planeRect[0]) planeRect[0] = vertData[i+0];
    if (vertData[i+1] < planeRect[1]) planeRect[1] = vertData[i+1];
    if (vertData[i+8] > planeRect[2]) planeRect[2] = vertData[i+8];
    if (vertData[i+9] > planeRect[3]) planeRect[3] = vertData[i+9];
  }
  // update border vert data
  const float backVertData[borderVertDataCount] = {
    // inside border
    // bl
    planeRect[0]+fingerUnit, planeRect[1]+fingerUnit,
    uitex_borderColor_x, uitex_borderColor_y,
    // tl
    planeRect[0]+fingerUnit, planeRect[3]-fingerUnit,
    uitex_borderColor_x, uitex_borderColor_y,
    // tr
    planeRect[2]-fingerUnit, planeRect[3]-fingerUnit,
    uitex_borderColor_x, uitex_borderColor_y,
    // br
    planeRect[2]-fingerUnit, planeRect[1]+fingerUnit,
    uitex_borderColor_x, uitex_borderColor_y,
    // outside border
    // bl
    planeRect[0], planeRect[1],
    uitex_borderColor_x, uitex_borderColor_y,
    // tl
    planeRect[0], planeRect[3],
    uitex_borderColor_x, uitex_borderColor_y,
    // tr
    planeRect[2], planeRect[3],
    uitex_borderColor_x, uitex_borderColor_y,
    // br
    planeRect[2], planeRect[1],
    uitex_borderColor_x, uitex_borderColor_y
  };
  fr(i,borderVertDataCount) {
    vertData[borderVertDataCount+i] = backVertData[i];
  }
}

void loadProgramHeader(programFileHeader *pgf, const char *path) {
  *pgf = pretendProgramFileHeader;
}
void loadProgramData(nodeDataOnDisk *ndod, const char *path) {
  fr(i, pretendProgramFileHeader.nodeDataCount) {ndod[i] = pretendData[i];}
}



void initUi(float videoSize[2]) {
  fr(i,2) {halfVideoSize[i] = videoSize[i]/2.0f;}
  glGenVertexArrays(1, &vao);_glec
  glBindVertexArray(vao);_glec
  glGenBuffers(1, &vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, vbo);_glec
  glGenBuffers(1, &ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);_glec
  glClearColor(uitex_clearR, uitex_clearG, uitex_clearB, uitex_clearA);
  
  const char *programPath = "pretendFile.punk";
  programFileHeader pgf = {0};
  loadProgramHeader(&pgf, programPath);
  nodeDataOnDisk ndod[pgf.nodeDataCount];
  loadProgramData(ndod, programPath);
  
  int ndcount     = pgf.nodeDataCount;
  planeElemCount  = pgf.planeElemCount;
  planeElemCap = nextHighestPO2(planeElemCount);
  resizeBuffers();
  nodeDef nddef = {{0}};
  int planeElemi = 0;
  float *peVertData = &vertData[peVertDataStart];
  float nodeBasePos[2] = {0};
  float destPos[2]     = {0};
  float destRect[4]    = {0};
  float srcRect[4]     = {0};
  for (int ndodi = 0; ndodi < ndcount; ndodi += nddef.ndodCount) {
    int nid = ndod[ndodi].n;
    getNodeDef(&nddef, nid);
    switch(nid) {
      case nid_add:
      case nid_sub:
      case nid_mul:
      case nid_div:
      case nid_output:
        // node face
        fr(i,2) {destPos[i] = nodeBasePos[i] = ndod[ndodi+1+i].p;}
        fr(i,4) {srcRect[i] = uitex_nodeFaces[nid*4 + i];}
        mapTexRectToVertPos(&peVertData[planeElemi*16], destPos, srcRect);
        planeElems[planeElemi].nbase.pei = pei_nface;
        planeElems[planeElemi].nbase.inletCount = nddef.inletCount;
        planeElems[planeElemi].nbase.nid = nid;
        planeElemi++;
        // inlets
        fr(i, nddef.inletCount) {
          destPos[0] = nodeBasePos[0] + nddef.inletPos[i]*fingerUnit;
          destPos[1] = nodeBasePos[1] - uitex_portH;
          fr(j,4) {srcRect[j] = uitex_inletRects[nddef.inTypes[i]*4 + j];}
          mapTexRectToVertPos(&peVertData[planeElemi*16], destPos, srcRect);
          planeElems[planeElemi].inlet.pei    = pei_inlet;
          planeElems[planeElemi].inlet.index  = i;
          planeElems[planeElemi].inlet.type   = nddef.inTypes[i];
          planeElems[planeElemi].inlet.conode = ndod[ndodi+ndodChildStart+i].c;
          planeElemi++;
        }
        break;
      case nid_numlit_b10w08:
        fr(i,2) {destPos[i] = destRect[i] = ndod[ndodi+1+i].p;}
        fr(i,2) {destRect[i+2] = destRect[i] + nddef.size[i];}
        srcRect[0] = srcRect[2] = uitex_numLitBackcolor_x;
        srcRect[1] = srcRect[3] = uitex_numLitBackcolor_y;
        mapTexRectToVertRect(&peVertData[planeElemi*16], destRect, srcRect);
        planeElems[planeElemi].numLit.pei   = pei_numLit;
        planeElems[planeElemi].numLit.width = nddef.extraPECount;
        planeElems[planeElemi].numLit.base  = 10;
        planeElems[planeElemi].numLit.value = ndod[ndodi+ndodNumLitValStart].v;
        planeElemi++;
        // just fill it with eights for now
        uitex_nmrlRect(srcRect, 8);
        fr(i,8) {
          mapTexRectToVertPos(&peVertData[planeElemi*16], destPos, srcRect);
          planeElems[planeElemi].numeric.pei   = pei_numeric;
          planeElems[planeElemi].numeric.value = 8;
          destPos[0] += fingerUnit;
          planeElemi++;
        }
        break;
      default: _SHOULD_NOT_BE_HERE_;
    }
    if (planeElemi > planeElemCount+1) _SHOULD_NOT_BE_HERE_;
  }
  
  resetPlaneRect();
  
  
  
  // indx data
  const uint32_t backElems[borderIndxDataCount] = {
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
  };
  fr(i, borderIndxDataCount) {
    indxData[i+borderIndxDataStart] = backElems[i];
  }
  setRectElems(&indxData[gcIndxDataStart], gcIndxDataCount);
  setRectElems(&indxData[peVertDataStart], planeElemCap*6);
  
  
  glClearColor(uitex_clearR, uitex_clearG, uitex_clearB, uitex_clearA);
  GLuint uiShader = createShaderProgram(
    "src/vert.glsl",
    "src/frag.glsl",
    "uiShader"
  );
  glUseProgram(uiShader);_glec
  unif_scroll        = glGetUniformLocation(uiShader, "scroll");_glec
  unif_halfVideoSize = glGetUniformLocation(uiShader, "halfVideoSize");_glec
  glUniform2f(unif_scroll, 0, 0);_glec
  glUniform2f(unif_halfVideoSize, halfVideoSize[0], halfVideoSize[1]);_glec
  GLuint uiTex;
  glGenTextures(1, &uiTex);_glec
  glBindTexture(GL_TEXTURE_2D, uiTex);_glec
  texFromBmp(uitex_path);
  glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);_glec
  
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
  centered[0] =  cX - halfVideoSize[0];
  centered[1] = -cY + halfVideoSize[1];
}

void clickDn(int posX, int posY) {
  corneredToCentered(newCurs_3, posX, posY);
  newCurs_3[2] = 1.0f;
  fr(i,3) {clickDnCurs_3[i] = newCurs_3[i];}
  fr(i,2) {clickDnScroll_2[i] = newScroll_2[i];}
  fr(i,2) {scrollVel_2[i] = 0;}
  onDrag    = onDragScroll;
  onClickUp = onClickUpScroll;
}
void curMove(int posX, int posY) {
  corneredToCentered(newCurs_3, posX, posY);
  if (newCurs_3[2]) onDrag(NULL);
}
void clickUp(int posX, int posY) {
  corneredToCentered(newCurs_3, posX, posY);
  newCurs_3[2] = 0;
  onClickUp(NULL);
}

float screenCrnrs_4[4]  = {0}; // xyxy, bl tr, relative to plane center
bool redrawPlane = true;

void perFrame(void) {
  fr(i,2) {newScroll_2[i] += scrollVel_2[i];}
  if (!allEq(newScroll_2, oldScroll_2, 2)) {
    screenCrnrs_4[0] = newScroll_2[0] - halfVideoSize[0];
    screenCrnrs_4[1] = newScroll_2[1] - halfVideoSize[1];
    screenCrnrs_4[2] = newScroll_2[0] + halfVideoSize[0];
    screenCrnrs_4[3] = newScroll_2[1] + halfVideoSize[1];
    fr(i,2) {
      if (screenCrnrs_4[i] < planeRect[i]) {
        newScroll_2[i] = planeRect[i] + halfVideoSize[i];
        scrollVel_2[i] = 0;
      }
      else if (screenCrnrs_4[i+2] > planeRect[i+2]) {
        newScroll_2[i] = planeRect[i+2] - halfVideoSize[i];
        scrollVel_2[i] = 0;
      }
    }
    redrawPlane = true;
  }
  //if (redrawPlane || redrawGc) {
    if (redrawPlane) {
      glClear(GL_COLOR_BUFFER_BIT);
      glUniform2f(unif_scroll, newScroll_2[0], newScroll_2[1]);_glec
      glDrawElements(
        GL_TRIANGLES,
        borderIndxDataCount + planeElemCount*6,
        GL_UNSIGNED_INT,
        (const GLvoid*)borderIndxDataStart
      );_glec
      redrawPlane = false;
    }
  //  drawGc();
  //}
  fr(i,3) {oldCurs_3[i] = newCurs_3[i];}
  fr(i,2) {oldScroll_2[i] = newScroll_2[i];}
  glFinish();
}


void exitUi(){
  //write back to disk before freeing
  free(planeElems);
}
