
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "misc.h"
#include "error.h"
#include "oglTools.h"
#include "planeElem.h"
#include "nodeDef.h"
#include "pretendFile.h"
#include "uitex.h"

float videoSize[2]     = {0};
float halfVideoSize[2] = {0};
void printVerts(const float *vertData, const int vertCount) {
  fr(i,vertCount) {
    printf(
      "%4i: vert pos: % 8.2f, % 8.2f  -  tex pos: % 8.2f, % 8.2f\n",
      i, vertData[4*i], vertData[4*i + 1], vertData[4*i + 2], vertData[4*i + 3]
    );
    if (!((i+1)%4)) puts("");
  }
}
void printIndxs(const uint32_t *indxData, const int indxCount) {
  for(int i = 0; i < indxCount; i += 6) {
    printf(
      "%4i: % 5i, % 5i, % 5i,  % 5i, % 5i, % 5i\n", i/6,
      indxData[i  ], indxData[i+1], indxData[i+2],
      indxData[i+3], indxData[i+4], indxData[i+5]
    );
  }
}
void printVertRect(const float vertData[16]) {
  printf(
    "TL: v: % 8.2f, % 8.2f  t: % 8.2f, % 8.2f      TR: v: % 8.2f, % 8.2f  t: % 8.2f, % 8.2f\n"
    "BL: v: % 8.2f, % 8.2f  t: % 8.2f, % 8.2f      BR: v: % 8.2f, % 8.2f  t: % 8.2f, % 8.2f\n",
    vertData[4],vertData[5], vertData[6],vertData[7],  vertData[8],vertData[9],   vertData[10],vertData[11],
    vertData[0],vertData[1], vertData[2],vertData[3],  vertData[12],vertData[13], vertData[14],vertData[15]
  );
}
void printRect(const float rect[4]) {
  printf(
    "(% 8.2f, % 8.2f) to (% 8.2f, % 8.2f)\n",
    rect[0],rect[1], rect[2],rect[3]
  );
}
void moveVertRect(float vertRect[16], const float offset[2]) {
  vertRect[ 0] += offset[0]; vertRect[ 1] += offset[1];
  vertRect[ 4] += offset[0]; vertRect[ 5] += offset[1];
  vertRect[ 8] += offset[0]; vertRect[ 9] += offset[1];
  vertRect[12] += offset[0]; vertRect[13] += offset[1];
}
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
void shiftTexRectV(float vertData[16], const int move) {
  vertData[ 3] += move;
  vertData[ 7] += move;
  vertData[11] += move;
  vertData[15] += move;
}
void setRectElems(
  uint32_t      *elems,
  const uint32_t elemsSize,
  const uint32_t vStart
) {
  uint32_t v = vStart;
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

#define gcButtonCount  10
#define gcButtonSide   ((uitex_gcRect[3] - uitex_gcRect[1])/2)
float   gcRect[4] = {0,0,0,0};
enum gcButtonIds {
  gcbi_play,
  gcbi_step,
  gcbi_compile,
  gcbi_zoomVideo,
  gcbi_zoomPlane,
  gcbi_branch,
  gcbi_lock,
  gcbi_revert,
  gcbi_save,
  gcbi_exit,
  gcbi_none
};
int gcbiPressed = -1;
bool togPlaying     = false;
bool togActOnBranch = false;
bool togLocked      = false;
bool menuOpenZoomVideo = false;
bool menuOpenZoomPlane = false;
bool menuOpenRevert    = false;


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
#define peIndxDataStart     (borderIndxDataStart + borderIndxDataCount)
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
  float *peVertData = &vertData[peVertDataStart];
  // start with the position of the first pe, doesn't matter what pe it is
  planeRect[0] = peVertData[0];
  planeRect[1] = peVertData[1];
  planeRect[2] = peVertData[8];
  planeRect[3] = peVertData[9];
  // then expand to include every other pe
  for (int i = 0; i < planeElemCount*16; i += 16) {
    if (peVertData[i  ] < planeRect[0]) planeRect[0] = peVertData[i  ];
    if (peVertData[i+1] < planeRect[1]) planeRect[1] = peVertData[i+1];
    if (peVertData[i+8] > planeRect[2]) planeRect[2] = peVertData[i+8];
    if (peVertData[i+9] > planeRect[3]) planeRect[3] = peVertData[i+9];
  }
  // add padding
  planeRect[0] -= planePadding;
  planeRect[1] -= planePadding;
  planeRect[2] += planePadding;
  planeRect[3] += planePadding;
  // make sure it's at least as big as the screen
  fr(i,2) {
    if (planeRect[i+2] - planeRect[i] < videoSize[i]) {
      float compensation = ceil((videoSize[i]-(planeRect[i+2]-planeRect[i]))/2);
      planeRect[i  ] -= compensation;
      planeRect[i+2] += compensation;
    }
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
    vertData[borderVertDataStart+i] = backVertData[i];
  }
}

void loadProgramHeader(programFileHeader *pgf, const char *path) {
  *pgf = pretendProgramFileHeader;
}
void loadProgramData(nodeDataOnDisk *ndod, const char *path) {
  fr(i, pretendProgramFileHeader.nodeDataCount) {ndod[i] = pretendData[i];}
}



void initUi() {
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
  float *peVertData = &vertData[peVertDataStart];
  {
    nodeDef nddef = {{0}}; // {{}}??
    int planeElemi = 0;
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
          // outlet
          if (nddef.outletCount) {
            destRect[0] = nodeBasePos[0];
            destRect[1] = nodeBasePos[1] + nddef.size[1];
            destRect[2] = destRect[0] + uitex_portW;
            destRect[3] = destRect[1] + uitex_portH;
            srcRect[0]  = uitex_inletRects[nddef.outType*4    ];
            srcRect[1]  = uitex_inletRects[nddef.outType*4 + 3];
            srcRect[2]  = uitex_inletRects[nddef.outType*4 + 2];
            srcRect[3]  = uitex_inletRects[nddef.outType*4 + 1];
            mapTexRectToVertRect(&peVertData[planeElemi*16], destRect, srcRect);
            planeElems[planeElemi].outlet.pei  = pei_outlet;
            planeElems[planeElemi].outlet.type = nddef.outType;
            // outlet.conode is set with the corresponding inlet
            planeElemi++;
          }
          // inlets
          fr(i, nddef.inletCount) {
            destPos[0] = nodeBasePos[0] + nddef.inletPos[i]*fingerUnit;
            destPos[1] = nodeBasePos[1] - uitex_portH;
            fr(j,4) {srcRect[j] = uitex_inletRects[nddef.inTypes[i]*4 + j];}
            mapTexRectToVertPos(&peVertData[planeElemi*16], destPos, srcRect);
            planeElems[planeElemi].inlet.pei    = pei_inlet;
            planeElems[planeElemi].inlet.index  = i;
            planeElems[planeElemi].inlet.type   = nddef.inTypes[i];
            int conode = ndod[ndodi + ndodChildStart + i].c;
            planeElems[planeElemi].inlet.conode = conode;
            planeElems[conode].outlet.conode = planeElemi;
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
  }
  // recenter all planeElems if significant drifting has occurred
  {
    float meanPos[2] = {0};
    fr(i,planeElemCount) {
      meanPos[0] += peVertData[i*16];
      meanPos[1] += peVertData[i*16+1];
      meanPos[0] += peVertData[i*16+8];
      meanPos[1] += peVertData[i*16+9];
    }
    const float offset[2] = {
      floor(-meanPos[0] / (planeElemCount*2)),
      floor(-meanPos[1] / (planeElemCount*2))
    };
    const float thresh = 5555; // arbitrary
    if (fabs(offset[0]) > thresh || fabs(offset[1]) > thresh) {
      fr(i,planeElemCount) {moveVertRect(&peVertData[i*16], offset);}
    }
  }
  
  resetPlaneRect();
  
  // global controls vertex data initialization
  {
    gcRect[0] = halfVideoSize[0] - (uitex_gcRect[2] - uitex_gcRect[0]);
    gcRect[1] = halfVideoSize[1] - gcButtonSide;
    gcRect[2] = halfVideoSize[0];
    gcRect[3] = halfVideoSize[1];
    float destPos[2] = {gcRect[0], gcRect[1]};
    float srcRect[4] = {
      uitex_gcRect[0],
      uitex_gcRect[1],
      uitex_gcRect[0] + gcButtonSide,
      uitex_gcRect[1] + gcButtonSide
    };
    float *gcVertData = &vertData[gcVertDataStart];
    fr(i, gcButtonCount) {
      mapTexRectToVertPos(&gcVertData[i*16], destPos, srcRect);
      destPos[0] += gcButtonSide;
      srcRect[0] += gcButtonSide;
      srcRect[2] += gcButtonSide;
    }
  }
  
  
  
  // indx data
  setRectElems(&indxData[peIndxDataStart], planeElemCap*6, peVertDataStart/4);
  setRectElems(&indxData[gcIndxDataStart], gcIndxDataCount, gcVertDataStart);
  {
    const uint32_t backElems[borderIndxDataCount] = {
      4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3
    };
    fr(i, borderIndxDataCount) {
      indxData[borderIndxDataStart+i] = borderVertDataStart/4 + backElems[i];
    }
  }
  
  #if LOG_UI_BUFFERS
  puts("\n\n\tvertData");
  puts("\nglobal controls");
  printVerts(&vertData[gcVertDataStart], gcVertDataCount/4);
  puts("\nborder");
  printVerts(&vertData[borderVertDataStart], borderVertDataCount/4);
  puts("\nplaneElems");
  printVerts(&vertData[peVertDataStart], planeElemCount*4);
  puts("\n\n\tindxData");
  puts("\nglobal controls");
  printIndxs(&indxData[gcIndxDataStart], gcIndxDataCount);
  puts("\nborder");
  printIndxs(&indxData[borderIndxDataStart], borderIndxDataCount);
  puts("\nplaneElems");
  printIndxs(&indxData[peIndxDataStart], planeElemCount*6);
  #endif
  
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

float newCurs_3[3]       = {0,0,0}; // cursor state relative to screen
float oldCurs_3[3]       = {0,0,0};
float clickDnCurs_3[3]   = {0,0,0};
float newScroll_2[2]     = {0,0}; // plane center to screen center difference
float oldScroll_2[2]     = {1,1}; // dif to trigger scroll on start
float clickDnScroll_2[2] = {0,0};
float scrollVel_2[2]     = {0,0};

bool redrawPlane = true;
bool redrawGc    = true;

typedef void (*cursEventHandler)(void *data);
void doNothing(void *data) {}
void onClickDnMain(void *data);
cursEventHandler onClickDn = onClickDnMain;
cursEventHandler onDrag    = doNothing;
cursEventHandler onClickUp = doNothing;



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
bool pointIsInRect(const float r[4], const float p[2]) {
  return (p[0] >= r[0] && p[0] < r[2]) && (p[1] >= r[1] && p[1] < r[3]);
}

void onClickUpReleaseGcb(void *data) {
  switch (gcbiPressed) {
    case gcbi_play:
    case gcbi_branch:
    case gcbi_lock:
    case gcbi_zoomVideo:
    case gcbi_zoomPlane:
    case gcbi_revert:
      break;
    case gcbi_step:
    case gcbi_compile:
    case gcbi_save:
    case gcbi_exit:
      shiftTexRectV(
        &vertData[gcVertDataStart + gcbiPressed*16],
        -gcButtonSide
      );
      redrawGc = true;
      break;
    default: _SHOULD_NOT_BE_HERE_;
  }
  gcbiPressed = gcbi_none;
  onClickUp   = doNothing;
}

void onClickDnZoomVideoMenuOpen(void *data) {
  // if the menu was clicked, then do menu action
  // otherwise...
  shiftTexRectV(&vertData[gcVertDataStart + gcbi_zoomVideo*16], -gcButtonSide);
  menuOpenZoomVideo = false;
  gcbiPressed = gcbi_none;
  redrawGc = true;
  onClickDn = onClickDnMain;
  onClickDnMain(NULL);
}
void onClickDnZoomPlaneMenuOpen(void *data) {
  // if the menu was clicked, then do menu action
  // otherwise...
  shiftTexRectV(&vertData[gcVertDataStart + gcbi_zoomPlane*16], -gcButtonSide);
  menuOpenZoomPlane = false;
  gcbiPressed = gcbi_none;
  redrawGc = true;
  onClickDn = onClickDnMain;
  onClickDnMain(NULL);
}
void onClickDnRevertMenuOpen(void *data) {
  // if the menu was clicked, then do menu action
  // otherwise...
  shiftTexRectV(&vertData[gcVertDataStart + gcbi_revert*16], -gcButtonSide);
  menuOpenRevert = false;
  gcbiPressed = gcbi_none;
  redrawGc = true;
  onClickDn = onClickDnMain;
  onClickDnMain(NULL);
}

void onClickDnMain(void *data) {
  if (pointIsInRect(gcRect, clickDnCurs_3)) {
    gcbiPressed = (clickDnCurs_3[0] - gcRect[0])/gcButtonSide;
    float texShift = 0;
    switch (gcbiPressed) {
      case gcbi_play:
        if (togPlaying) {togPlaying = false; texShift = -gcButtonSide;}
        else            {togPlaying = true;  texShift =  gcButtonSide;}
        onClickUp = onClickUpReleaseGcb;
        break;
      case gcbi_branch:
        if (togActOnBranch) {togActOnBranch = false; texShift = -gcButtonSide;}
        else                {togActOnBranch = true;  texShift =  gcButtonSide;}
        onClickUp = onClickUpReleaseGcb;
        break;
      case gcbi_lock:
        if (togLocked) {togLocked = false; texShift = -gcButtonSide;}
        else           {togLocked = true;  texShift =  gcButtonSide;}
        onClickUp = onClickUpReleaseGcb;
        break;
      case gcbi_zoomVideo:
        if (menuOpenZoomVideo) _SHOULD_NOT_BE_HERE_;
        menuOpenZoomVideo = true; texShift = gcButtonSide;
        onClickDn = onClickDnZoomVideoMenuOpen;
        break;
      case gcbi_zoomPlane:
        if (menuOpenZoomPlane) _SHOULD_NOT_BE_HERE_;
        menuOpenZoomPlane = true; texShift = gcButtonSide;
        onClickDn = onClickDnZoomPlaneMenuOpen;
        break;
      case gcbi_revert:
        if (menuOpenRevert)    _SHOULD_NOT_BE_HERE_;
        menuOpenRevert    = true; texShift = gcButtonSide;
        onClickDn = onClickDnRevertMenuOpen;
        break;
      case gcbi_step:
        if (togPlaying) {
          togPlaying = false;
          shiftTexRectV(
            &vertData[gcVertDataStart + gcbi_play*16],
            -gcButtonSide
          );
        }
      case gcbi_compile:
      case gcbi_save:
      case gcbi_exit:
        texShift  = gcButtonSide;
        onClickUp = onClickUpReleaseGcb;
        break;
      default: _SHOULD_NOT_BE_HERE_;
    }
    shiftTexRectV(&vertData[gcVertDataStart + gcbiPressed*16], texShift);
    onDrag    = doNothing;
    redrawGc  = true;
  }
  else {
    fr(i,2) {clickDnScroll_2[i] = newScroll_2[i];}
    fr(i,2) {scrollVel_2[i] = 0;}
    onDrag    = onDragScroll;
    onClickUp = onClickUpScroll;
  }
}

void clickDn(int posX, int posY) {
  corneredToCentered(newCurs_3, posX, posY);
  newCurs_3[2] = 1.0f;
  fr(i,3) {clickDnCurs_3[i] = newCurs_3[i];}
  onClickDn(NULL);
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


void perFrame(const uint32_t curFrame) {
  fr(i,2) {
    newScroll_2[i] += scrollVel_2[i];
    if (newScroll_2[i] != oldScroll_2[i]) {
      if (newScroll_2[i] > -planeRect[i] - halfVideoSize[i]) {
        newScroll_2[i] = -planeRect[i] - halfVideoSize[i];
      }
      else if (newScroll_2[i] < -planeRect[i+2] + halfVideoSize[i]) {
        newScroll_2[i] = -planeRect[i+2] + halfVideoSize[i];
      }
      redrawPlane = true;
    }
  }
  if (redrawPlane || redrawGc) {
    if (redrawPlane) {
      glClear(GL_COLOR_BUFFER_BIT);
      glUniform2f(unif_scroll, newScroll_2[0], newScroll_2[1]);_glec
      glDrawElements(
        GL_TRIANGLES,
        borderIndxDataCount + planeElemCount*6,
        GL_UNSIGNED_INT,
        (const GLvoid*) (borderIndxDataStart*sizeof(uint32_t))
      );_glec
      redrawPlane = false;
    }
    glUniform2f(unif_scroll, 0, 0);_glec
    glDrawElements(
      GL_TRIANGLES,
      gcIndxDataCount,
      GL_UNSIGNED_INT,
      (const GLvoid*) (gcIndxDataStart*sizeof(uint32_t))
    );_glec
    redrawGc = false;
  }
  fr(i,3) {oldCurs_3[i] = newCurs_3[i];}
  fr(i,2) {oldScroll_2[i] = newScroll_2[i];}
  glFinish();
}


void exitUi() {
  // write back to disk before freeing
  free(planeElems);
}
