

#include "planeElem.h"
#include "pretendFile.h"

const uint32_t fingerUnit = 16;
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
  const float destPos_px[2],     // bottom left position pixels
  const float srcRect_nt[4]   // normalized texture coordinates
) {
  // bl
  destVertData[ 0] = destPos_px[0];
  destVertData[ 1] = destPos_px[1];
  destVertData[ 2] = srcRect_nt[0];
  destVertData[ 3] = srcRect_nt[1];
  // tl
  destVertData[ 4] = destPos_px[0];
  destVertData[ 5] = destPos_px[1] + (srcRect_nt[3] - srcRect_nt[1]);
  destVertData[ 6] = srcRect_nt[0];
  destVertData[ 7] = srcRect_nt[3];
  // tr
  destVertData[ 8] = destPos_px[0] + (srcRect_nt[2] - srcRect_nt[0]);
  destVertData[ 9] = destVertData[5];
  destVertData[10] = srcRect_nt[2];
  destVertData[11] = srcRect_nt[3];
  // br
  destVertData[12] = destVertData[8];
  destVertData[13] = destPos_px[1];
  destVertData[14] = srcRect_nt[2];
  destVertData[15] = srcRect_nt[1];
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

const uint32_t gcButtonCount       = 10;
const uint32_t gcVertDataStart     =  0;
const uint32_t gcVertDataCount     = gcButtonCount*16;
const uint32_t borderVertDataStart = gcVertDataStart + gcVertDataCount;
const uint32_t borderVertDataCount = 32;
// there will be more fixed-size vert data later
const  uint32_t peVertDataStart = borderVertDataStart + borderVertDataCount;
inline uint32_t peVertDataCap(void) {return planeElemCap*12 - peVertDataStart;}
inline uint32_t lineVertDataStart(void) {
  return peVertDataStart + peVertDataCap();
}
inline uint32_t lineVertDataCap(void) {return planeElemCap*4;}
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
      bufSize,
      0,
      bufferStorageFlags
    );_glec
    vertData = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufSize, bufferStorageFlags);_glec
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
  }
  else {
    if (!indxData) _SHOULD_NOT_BE_HERE_;
    // copy data to new, bigger buffers
  }
}


const float planePadding = fingerUnit*12; // arbitrary
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
  // then add padding
  planeRect[0] -= planePadding;
  planeRect[1] -= planePadding;
  planeRect[2] += planePadding;
  planeRect[3] += planePadding;
  // make sure it's big enough to fill the screen
  const float planeSize[2] = {
    planeRect[2] - planeRect[0],
    planeRect[3] - planeRect[1]
  };
  if (planeSize[0] < videoSize_px2[0]) {
    planeRect[0] -= videoSize_px2[0] - planeSize[0];
    planeRect[2] += videoSize_px2[0] - planeSize[0];
  }
  if (planeSize[1] < videoSize_px2[1]) {
    planeRect[1] -= videoSize_px2[1] - planeSize[1];
    planeRect[3] += videoSize_px2[1] - planeSize[1];
  }
  // update border vert data
  const float backVertData[borderVertDataCount] = {
    // inside border
    // bl
    planeRect[0]+fingerUnit, planeRect[1]+fingerUnit,
    uitex_borderColor, uitex_borderColor,
    // tl
    planeRect[0]+fingerUnit, planeRect[3]-fingerUnit,
    uitex_borderColor, uitex_borderColor,
    // tr
    planeRect[2]-fingerUnit, planeRect[3]-fingerUnit,
    uitex_borderColor, uitex_borderColor,
    // br
    planeRect[2]-fingerUnit, planeRect[1]+fingerUnit,
    uitex_borderColor, uitex_borderColor,
    // outside border
    // bl
    planeRect[0], planeRect[1],
    uitex_borderColor, uitex_borderColor,
    // tl
    planeRect[0], planeRect[3],
    uitex_borderColor, uitex_borderColor,
    // tr
    planeRect[2], planeRect[3],
    uitex_borderColor, uitex_borderColor,
    // br
    planeRect[2], planeRect[1],
    uitex_borderColor, uitex_borderColor,
  };
  fr(i,borderVertDataCount) {
    vertData[borderVertDataCount+i] = backVertData[i];
  }
}


void loadProgram(
  nodeDataOnDisk   *ndod,
  programFileHeader pgf,
  const char       *path
) {
  pgf = pretendProgramFileHeader;
  ndod = malloc(sizeof(nodeDataOnDisk)*pgf.nodeDataCount);
  fr(i,pgf.nodeDataCount) {ndod[i] = pretendData[i]}
}


void initUi(float videoSize_px2[2]) {
  fr(i,2) {halfVideoSize_2[i] = videoSize_px2[i]/2.0f;}
  glGenVertexArrays(1, &vao);_glec
  glBindVertexArray(vao);_glec
  glGenBuffers(1, &vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, vbo);_glec
  glGenBuffers(1, &ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);_glec
  
  nodeDataOnDisk *ndod = NULL;
  programFileHeader pgf = {0};
  loadProgram(ndod, &pgf, "pretendFile.punk");
  int     ndcount = programFileHeader.nodeDataCount;
  nodeDef nddef   = {0};
  for (int i = 0; i < ndcount; i += nddef.ndodCount) {
    getNodeDef(&nddef, ndod[i].n);
    planeElemCount += 1/*face*/ + nddef.inletCount + nddef.extraPECount;
  }
  planeElemCap = nextHighestPO2(planeElemCount);
  resizeBuffers();
  for (
    int ndodi = 0, int planeElemi;
    ndodi < ndcount;
    ndodi += nddef.ndodCount, planeElemi++
  ) {
    nodeId nid = ndod[ndodi].n;
    getNodeDef(&nddef, nid);
    float *peVertData = &vertData[peVertDataStart];
    float destPos_px[2] = {0};
    float srcRect_nt[4] = {0};
    switch(nid) {
      case nid_output:
      case nid_add:
      case nid_sub:
      case nid_mul:
      case nid_div:
        fr(i,2) {destPos_px[i] = ndod[ndodi+1+i].p;}
        fr(i,4) {srcRect_nt[i] = uitex_nodeFaces[planeElemi*4 + i];}
        mapTexRectToVerts(&peVertData[planeElemi*16], destPos_px, srcRect_nt);
        break;
      case nid_numlit7:
      default:_SHOULD_NOT_BE_HERE_;
    }
  }
  free(ndod);
  
  resetPlaneRect(void);
  
  GLuint uiShader;
  GLuint uiTex;
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
      if (screenCrnrs_4[i] < planeRect[i]) {
        newScroll_2[i] = planeRect[i] + halfVideoSize_2[i];
        scrollVel_2[i] = 0;
      }
      else if (screenCrnrs_4[i+2] > planeRect[i+2]) {
        newScroll_2[i] = planeRect[i+2] - halfVideoSize_2[i];
        scrollVel_2[i] = 0;
      }
    }
    redrawPlane = true;
  }
  if (redrawPlane || redrawGc) {
    if (redrawPlane) {
      glBindVertexArray(vao);_glec
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


void exitUi(){
  //write back to disk before freeing
  free(planeElems);
}
