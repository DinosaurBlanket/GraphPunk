
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "error.h"
#include "module.h"
#include "forUi.h"

//on disk there is:
//  a folder of module files named after their moduleID,
//  a list of moduleIDs that are in use,
//  maybe a file containing global state such as the last module they were in


typedef struct {
  uint32_t      moduleId;
  uint32_t      parentId;
  uint32_t      planeElemCount;
  float         planePos[2];
  moduleToggles toggles;
} diskModuleHeader;
//There are two chunks of data following the header, 
//  the planeElems(nodes and ports), same as in-memory
//  the positions(xy, bl) of each planeElem, 1/8 of what's in-memory(vertData)



// temorary declarations for data that will later come from a file
const diskModuleHeader lastModHeader = {
  .moduleId       = rootModId,
  .parentId       = 0,
  .planeElemCount = 4,
  .planePos       = {0,0}
};
const planeElem lastModPlaneElems[4] = {
  {
    .n.pei       = pei_aface,
    .n.oputCount = 1,
    .n.iputCount = 2,
    .n.lineVerts = 0,
    .n.id        = aid_add,
    .n.module    = NULL
  },
  {
    .p.pei  = pei_oport,
    .p.type = dt_n,
    .p.pos  = 0,
    .p.node = 0,
    .p.ocon = 0
  },
  {
    .p.pei  = pei_iport,
    .p.type = dt_n,
    .p.pos  = 0,
    .p.node = 0,
    .p.ocon = 0
  },
  {
    .p.pei  = pei_iport,
    .p.type = dt_n,
    .p.pos  = 1,
    .p.node = 0,
    .p.ocon = 0
  }
};
const float lastModPlaneElemPositions[] = {
  16,16,
  16,32,
  16, 0,
  32, 0
};







module root = {
  .id       = rootModId,
  .parent   = NULL,
  .toggles  = {0},
  .faceRect = {0}
};

module *loadModules(uint32_t id) {
  module *m;
  if (id == rootModId) m = &root;
  else {
    // m = malloc module
    // load module data from disk
    // initialize id, parent, toggles, faceRect
  }
  plane *p = &m->plane;
  
  // initialize some critical plane data
  p->planeElemCount = lastModHeader.planeElemCount;
  if (!lastModHeader.planeElemCount) {
    p->planeElemCap = 0;
    p->planeElems   = NULL;
    return m;
  }
  p->planeElemCap = nextHighestPO2(lastModHeader.planeElemCount);
  
  uint32_t lineCount = 0;
  fr(i, p->planeElemCount) {
    if (p->planeElems[i].pei == pei_iport && p->planeElems[i].p.ocon) {
      lineCount++;
    }
  }
  p->nodeVertsOffset = initialNodeVertsOffset;
  while (
    16*lineCount         > p->nodeVertsOffset - backVertsSize ||
    16*p->planeElemCount > p->nodeVertsOffset
  ) {
    p->nodeVertsOffset = nextHighestPO2(p->nodeVertsOffset);
  }
  // first 1/2 of data is for background and lines, 2nd 1/2 is for planeElems
  const uint32_t bufSize = 2 * p->nodeVertsOffset * sizeof(float);
  
  glGenVertexArrays(1, &p->vao);_glec
  glBindVertexArray(p->vao);_glec
  glGenBuffers(1, &p->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, p->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, p->vbo);_glec
  glBufferStorage(GL_ARRAY_BUFFER, bufSize, 0, bufferStorageFlags);_glec
  
  p->vertData = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufSize, bufferStorageFlags);_glec
  p->planeElems = malloc(p->planeElemCap*sizeof(planeElem));
  
  fr(i, p->planeElemCount) {
    p->planeElems[i] = lastModPlaneElems[i];
    if (p->planeElems[i].pei == pei_mface) {
      p->planeElems[i].n.module = (struct module*)loadModules(p->planeElems[i].n.id); // why the hell do i need to cast this??
    }
    p->vertData[16*i    ] = lastModPlaneElemPositions[2*i];
    p->vertData[16*i + 1] = lastModPlaneElemPositions[2*i + 1];
  }
  
  return m;
}

// returns module you were in you last closed ...for now that will be root
module *saveLoadInit(void) {
  loadModules(rootModId);
  return &root;
}

void save(module *curModule) {}

void saveLoadExit(module *curModule) {
  // traverse module tree and free from bottom up
}
