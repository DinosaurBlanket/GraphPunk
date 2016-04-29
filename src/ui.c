

#include "planeElem.h"
#include "pretendFile.h"

#define fingerUnit 16
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



float      planeRect[4]   = {0};
float      planePos[2]    = {0};
planeElem *planeElems     = NULL;
uint32_t   planeElemCount = 0;
uint32_t   planeElemCap   = 0;
float     *vertData       = NULL; // GL buffer, 16 floats(4 verts) per rect
uint32_t   vertDataCount  = 0;;   // float count
uint32_t  *indxData       = NULL; // GL buffer,  6 ints per rect
uint32_t   indxDataCount  = 0;    // int count
GLuint vao = 0;
GLuint vbo = 0;
GLuint ebo = 0;
GLint unif_scroll = 0;
GLint unif_halfVideoSize = 0;

#define gcVertDataStart  0
#define gcButtonCount   10
#define gcVertDataCount (gcButtonCount*16)
// there will be more fixed-size vert data later
#define peVertDataStart   gcVertDataCount;
#define peVertDataCount   ((vertDataCount/4)*3 - peVertDataStart)
#define lineVertDataStart (peVertDataStart + peVertDataCount)
#define lineVertDataCount (vertDataCount/4)

nodeDataOnDisk *ndod = NULL;
programFileHeader programFileHeader = {0};

void loadProgram(const char *path) {
  programFileHeader = pretendProgramFileHeader;
  ndod = malloc(sizeof(nodeDataOnDisk)*programFileHeader.nodeDataCount);
  fr(i,programFileHeader.nodeDataCount) {ndod[i] = pretendData[i]}
}


void initUi(float videoSize_px2[2]) {
  fr(i,2) {halfVideoSize_2[i] = videoSize_px2[i]/2.0f;}
  
  loadProgram("pretendFile.punk");
  int     ncount = programFileHeader.nodeDataCount;
  nodeDef ndef = {0};
  for(int i = 0; i < ncount;) {
    getNodeDef(&ndef, ndod[i])
    planeElemCount += 1/*face*/ + ndef.inletCount + def.extraPECount;
    i += 2/*position*/ + ndef.inletCount;
    switch(ndod[i]) {
      case nid_numlit7: i += 1; // value
    }
  }
  planeElemCap = nextHighestPO2(planeElemCount);
  planeElems = malloc(sizeof(planeElem)*planeElemCap);
  
  
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
  const GLbitfield bufferStorageFlags =
    GL_MAP_WRITE_BIT      |
    GL_MAP_PERSISTENT_BIT |
    GL_MAP_COHERENT_BIT
  ;
  glGenVertexArrays(1, &vao);_glec
  glBindVertexArray(vao);_glec
  glGenBuffers(1, &vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, vbo);_glec
  glBufferStorage(GL_ARRAY_BUFFER, bufSize, 0, bufferStorageFlags);_glec
  
  vertData = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufSize, bufferStorageFlags);_glec
  planeElems = malloc(planeElemCap*sizeof(planeElem));
}




void exitUi(){
  //write back to disk before freeing
  free(ndod);
}
